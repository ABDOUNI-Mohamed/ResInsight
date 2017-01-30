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

#include "RimSimWellFracture.h"

#include "RigCell.h"
#include "RigMainGrid.h"

#include "RimEclipseView.h"
#include "RimEclipseWell.h"
#include "RimProject.h"

#include "cafPdmUiDoubleSliderEditor.h"



CAF_PDM_SOURCE_INIT(RimSimWellFracture, "SimWellFracture");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSimWellFracture::RimSimWellFracture(void)
{
    CAF_PDM_InitObject("SimWellFracture", ":/FractureSymbol16x16.png", "", "");

    CAF_PDM_InitField(&m_location, "MeasuredDepth", 0.0f, "Measured Depth Location", "", "", "");
    m_location.uiCapability()->setUiEditorTypeName(caf::PdmUiDoubleSliderEditor::uiEditorTypeName());

    CAF_PDM_InitField(&m_branchIndex, "Branch", 0, "Branch", "", "", "");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSimWellFracture::~RimSimWellFracture()
{
}
 
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSimWellFracture::setClosestWellCoord(cvf::Vec3d& position, size_t branchIndex)
{
    updateBranchGeometry();

    double location = m_branchCenterLines[branchIndex].locationAlongWellCoords(position);

    m_branchIndex = static_cast<int>(branchIndex);

    m_location = location;
    updateFracturePositionFromLocation();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSimWellFracture::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    RimFracture::fieldChangedByUi(changedField, oldValue, newValue);

    if (   changedField == &m_location
        || changedField == &m_branchIndex
        )
    {
        updateFracturePositionFromLocation();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSimWellFracture::recomputeWellCenterlineCoordinates()
{
    m_branchCenterLines.clear();

    updateBranchGeometry();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSimWellFracture::updateFracturePositionFromLocation()
{
    updateBranchGeometry();

    if (m_branchCenterLines.size() > 0)
    {
        cvf::Vec3d interpolated = m_branchCenterLines[m_branchIndex()].interpolatedPointAlongWellPath(m_location());

        this->setAnchorPosition(interpolated);

        RimProject* proj;
        this->firstAncestorOrThisOfType(proj);
        if (proj) proj->createDisplayModelAndRedrawAllViews();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSimWellFracture::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    uiOrdering.add(&name);

    uiOrdering.add(&m_location);

    caf::PdmUiGroup* geometryGroup = uiOrdering.addNewGroup("Properties");
    geometryGroup->add(&azimuth);
    geometryGroup->add(&m_fractureTemplate);

    caf::PdmUiGroup* fractureCenterGroup = uiOrdering.addNewGroup("Fracture Center Info");
    fractureCenterGroup->add(&m_uiAnchorPosition);
    fractureCenterGroup->add(&m_displayIJK);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSimWellFracture::defineEditorAttribute(const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute * attribute)
{
    if (field == &m_location)
    {
        caf::PdmUiDoubleSliderEditorAttribute* myAttr = dynamic_cast<caf::PdmUiDoubleSliderEditorAttribute*>(attribute);

        if (myAttr)
        {
            updateBranchGeometry();

            if (m_branchCenterLines.size() > 0)
            {
                const RigSimulationWellCoordsAndMD& pointAndMd = m_branchCenterLines[m_branchIndex];

                myAttr->m_minimum = pointAndMd.measuredDepths().front();
                myAttr->m_maximum = pointAndMd.measuredDepths().back();
                myAttr->m_sliderTickCount = pointAndMd.measuredDepths().back();
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimSimWellFracture::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> options = RimFracture::calculateValueOptions(fieldNeedingOptions, useOptionsOnly);

    if (fieldNeedingOptions == &m_branchIndex)
    {
        if (m_branchCenterLines.size() == 0)
        {
            updateBranchGeometry();
        }

        if (m_branchCenterLines.size() > 0)
        {
            size_t branchCount = m_branchCenterLines.size();

            for (size_t bIdx = 0; bIdx < branchCount; ++bIdx)
            {
                // Use 1-based index in UI
                options.push_back(caf::PdmOptionItemInfo(QString::number(bIdx + 1), QVariant::fromValue(bIdx)));
            }
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSimWellFracture::updateBranchGeometry()
{
    if (m_branchCenterLines.size() == 0)
    {
        setBranchGeometry();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSimWellFracture::setBranchGeometry()
{
    m_branchCenterLines.clear();

    RimEclipseWell* rimWell = nullptr;
    this->firstAncestorOrThisOfType(rimWell);
    CVF_ASSERT(rimWell);

    std::vector< std::vector <cvf::Vec3d> > pipeBranchesCLCoords;
    std::vector< std::vector <RigWellResultPoint> > pipeBranchesCellIds;

    rimWell->calculateWellPipeStaticCenterLine(pipeBranchesCLCoords, pipeBranchesCellIds);

    for (const auto& branch : pipeBranchesCLCoords)
    {
        RigSimulationWellCoordsAndMD wellPathWithMD(branch);

        m_branchCenterLines.push_back(wellPathWithMD);
    }
}
