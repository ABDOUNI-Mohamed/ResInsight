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

#include "RimFractureTemplateCollection.h"

#include "RimFractureTemplate.h"
#include "RimStimPlanFractureTemplate.h"

#include "cafPdmObject.h"



CAF_PDM_SOURCE_INIT(RimFractureTemplateCollection, "FractureDefinitionCollection");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimFractureTemplateCollection::RimFractureTemplateCollection(void)
{
    CAF_PDM_InitObject("Fracture Templates", ":/FractureTemplate16x16.png", "", "");

    CAF_PDM_InitField(&isActive, "Active", true, "Active", "", "", "");
    
    CAF_PDM_InitFieldNoDefault(&fractureDefinitions, "FractureDefinitions", "", "", "", "");
    fractureDefinitions.uiCapability()->setUiHidden(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimFractureTemplateCollection::~RimFractureTemplateCollection()
{
    fractureDefinitions.deleteAllChildObjects();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFractureTemplateCollection::deleteFractureDefinitions()
{
    fractureDefinitions.deleteAllChildObjects();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFractureTemplateCollection::loadAndUpdateData()
{
    for (RimFractureTemplate* f : fractureDefinitions())
    {
        RimStimPlanFractureTemplate* stimPlanFracture = dynamic_cast<RimStimPlanFractureTemplate*>(f);
        if (stimPlanFracture)
        {
            stimPlanFracture->loadDataAndUpdate();
        }
    }
}

