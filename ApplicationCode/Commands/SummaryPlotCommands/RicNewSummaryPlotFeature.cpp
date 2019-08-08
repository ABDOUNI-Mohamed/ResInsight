/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-     Statoil ASA
// 
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
// 
//  ResInsight is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.
// 
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html> 
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#include "RicNewSummaryPlotFeature.h"

#include "RiaApplication.h"
#include "RiaPreferences.h"
#include "RiaSummaryTools.h"

#include "RicEditSummaryPlotFeature.h"
#include "RicSummaryCurveCreator.h"
#include "RicSummaryCurveCreatorDialog.h"

#include "RimEnsembleCurveFilter.h"
#include "RimEnsembleCurveFilterCollection.h"
#include "RimRegularLegendConfig.h"
#include "RimSummaryCurveFilter.h"
#include "RimSummaryPlot.h"
#include "RimSummaryPlotCollection.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseCollection.h"
#include "RimProject.h"
#include "RimSummaryCaseMainCollection.h"

#include "RiuPlotMainWindow.h"

#include "cvfAssert.h"
#include "cafSelectionManagerTools.h"

#include <QAction>


CAF_CMD_SOURCE_INIT(RicNewSummaryPlotFeature, "RicNewSummaryPlotFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicNewSummaryPlotFeature::isCommandEnabled()
{
    RimSummaryPlotCollection* sumPlotColl = nullptr;

    caf::PdmObject* selObj = dynamic_cast<caf::PdmObject*>(caf::SelectionManager::instance()->selectedItem());
    if (selObj)
    {
        sumPlotColl = RiaSummaryTools::parentSummaryPlotCollection(selObj);
    }

    auto ensembleFilter = dynamic_cast<RimEnsembleCurveFilter*>(selObj);
    auto ensembleFilterColl = dynamic_cast<RimEnsembleCurveFilterCollection*>(selObj);
    auto legendConfig = dynamic_cast<RimRegularLegendConfig*>(selObj);

    if (ensembleFilter || ensembleFilterColl || legendConfig) return false;
    if (sumPlotColl) return true;

    // Multiple case selections
    std::vector<caf::PdmUiItem*> selectedItems = caf::selectedObjectsByTypeStrict<caf::PdmUiItem*>();

    for (auto item : selectedItems)
    {
        if (!dynamic_cast<RimSummaryCase*>(item) && !dynamic_cast<RimSummaryCaseCollection*>(item))
            return false;
    }
    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewSummaryPlotFeature::onActionTriggered(bool isChecked)
{
    RimProject* project = RiaApplication::instance()->project();
    CVF_ASSERT(project);

    std::vector<RimSummaryCase*> selectedCases = caf::selectedObjectsByType<RimSummaryCase*>();
    std::vector<RimSummaryCaseCollection*> selectedGroups = caf::selectedObjectsByType<RimSummaryCaseCollection*>();

    std::vector<caf::PdmObject*> sourcesToSelect(selectedCases.begin(), selectedCases.end());

    if (sourcesToSelect.empty() && selectedGroups.empty())
    {
        const auto allSingleCases = project->firstSummaryCaseMainCollection()->topLevelSummaryCases();
        const auto allGroups = project->summaryGroups();
        std::vector<RimSummaryCaseCollection*> allEnsembles;
        for (const auto group : allGroups) if (group->isEnsemble()) allEnsembles.push_back(group);

        if (!allSingleCases.empty())
        {
            sourcesToSelect.push_back(allSingleCases.front());
        }
        else if (!allEnsembles.empty())
        {
            sourcesToSelect.push_back(allEnsembles.front());
        }
    }

    // Append grouped cases
    for (auto group : selectedGroups)
    {
        if (group->isEnsemble())
        {
            sourcesToSelect.push_back(group);
        }
        else
        {
            auto groupCases = group->allSummaryCases();
            sourcesToSelect.insert(sourcesToSelect.end(), groupCases.begin(), groupCases.end());
        }
    }

    auto dialog = RicEditSummaryPlotFeature::curveCreatorDialog();

    if (!dialog->isVisible())
    {
        dialog->show();
    }
    else
    {
        dialog->raise();
    }

    dialog->updateFromDefaultCases(sourcesToSelect);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewSummaryPlotFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Open Summary Plot Editor");
    actionToSetup->setIcon(QIcon(":/SummaryPlotLight16x16.png"));
}

#include "RiuPlotMainWindowTools.h"
#include "RicNewSummaryCurveFeature.h"
#include "RimMainPlotCollection.h"

CAF_CMD_SOURCE_INIT(RicNewSummaryPlotDirectFeature, "RicNewSummaryPlotDirectFeature");


auto extractSumPlotCollectionOrSelectedSumCasesFromSelection()
{
    std::vector<RimSummaryCase*> selectedSumCases;
    RimSummaryPlotCollection* sumPlotColl = nullptr;

    std::vector<caf::PdmUiItem*> selectedItems;
    caf::SelectionManager::instance()->selectedItems(selectedItems);

    if ( selectedItems.size() )
    {
        caf::PdmObject* selObj = dynamic_cast<caf::PdmObject*>(selectedItems[0]);
        sumPlotColl = RiaSummaryTools::parentSummaryPlotCollection(selObj);
    }

    if (!sumPlotColl)
    {
        caf::SelectionManager::instance()->objectsByTypeStrict(&selectedSumCases);
    }

    return std::make_pair(sumPlotColl, selectedSumCases);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicNewSummaryPlotDirectFeature::isCommandEnabled()
{
    auto sumPlotSumCasesPair = extractSumPlotCollectionOrSelectedSumCasesFromSelection();

    return sumPlotSumCasesPair.first || sumPlotSumCasesPair.second.size();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewSummaryPlotDirectFeature::onActionTriggered(bool isChecked)
{
    auto sumPlotSumCasesPair = extractSumPlotCollectionOrSelectedSumCasesFromSelection();

    RimSummaryPlotCollection* sumPlotColl = RiaApplication::instance()->project()->mainPlotCollection()->summaryPlotCollection();

    RimSummaryPlot* newPlot = nullptr;

    if (sumPlotSumCasesPair.first)
    {
        auto sumCaseVector = RiaApplication::instance()->project()->allSummaryCases();

        newPlot = sumPlotColl->createSummaryPlotWithAutoTitle();

        if (sumCaseVector.size())
        {
            RicNewSummaryCurveFeature::addCurveToPlot(newPlot, sumCaseVector[0]);
        }
    }
    else if (sumPlotSumCasesPair.second.size())
    {
        newPlot = sumPlotColl->createSummaryPlotWithAutoTitle();

        for (RimSummaryCase* sumCase : sumPlotSumCasesPair.second)
        {
            RicNewSummaryCurveFeature::addCurveToPlot(newPlot, sumCase);
        }
    }

    if ( newPlot )
    {
        newPlot->loadDataAndUpdate();

        sumPlotColl->updateConnectedEditors();

        RiuPlotMainWindowTools::setExpanded(newPlot);
        RiuPlotMainWindowTools::selectAsCurrentItem(newPlot);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewSummaryPlotDirectFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("New Summary Plot");
    actionToSetup->setIcon(QIcon(":/SummaryPlotLight16x16.png"));

}
