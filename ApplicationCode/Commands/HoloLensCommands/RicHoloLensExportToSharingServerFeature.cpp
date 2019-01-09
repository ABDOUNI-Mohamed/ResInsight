/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
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

#include "RicHoloLensExportToSharingServerFeature.h"
#include "RicHoloLensSessionManager.h"
#include "RicHoloLensSession.h"

#include "RiaApplication.h"
#include "RiaQIconTools.h"
#include "RiaLogging.h"

#include "RimGridView.h"

#include <QAction>

CAF_CMD_SOURCE_INIT(RicHoloLensExportToSharingServerFeature, "RicHoloLensExportToSharingServerFeature");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicHoloLensExportToSharingServerFeature::isCommandEnabled()
{
    RicHoloLensSession* session = RicHoloLensSessionManager::instance()->session();
    if (session && session->isSessionValid())
    {
        return true;
    }
    else
    {
        return false;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicHoloLensExportToSharingServerFeature::onActionTriggered(bool isChecked)
{
    RicHoloLensSession* session = RicHoloLensSessionManager::instance()->session();
    if (!session || !session->isSessionValid())
    {
        RiaLogging::error("No valid HoloLens session present");
        return;
    }

    RimGridView* activeView = RiaApplication::instance()->activeGridView();
    if (!activeView)
    {
        RiaLogging::error("No active view");
        return;
    }


    session->updateSessionDataFromView(*activeView);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicHoloLensExportToSharingServerFeature::setupActionLook(QAction* actionToSetup)
{
    QPixmap pixmap(":/hololens.png");
    QPixmap overlayPixmap(":/arrow-right-green.png");

    QPixmap combinedPixmap = RiaQIconTools::appendPixmapUpperLeft(pixmap, overlayPixmap);
    actionToSetup->setIcon(QIcon(combinedPixmap));

    actionToSetup->setText("Export to Sharing Server");
}
