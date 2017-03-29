/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 -     Statoil ASA
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

#pragma once

#include "cvfBase.h"
#include "cvfVector3.h"

#include <vector>

//==================================================================================================
///  
///  
//==================================================================================================
class RimStimPlanCell 
{

public:
    RimStimPlanCell();
    RimStimPlanCell(std::vector<cvf::Vec3d> polygon, double value, size_t i, size_t j);


    virtual ~RimStimPlanCell();

    std::vector<cvf::Vec3d> getPolygon() { return m_polygon; }
    double                  getValue() { return m_value; }
    size_t                  getI() { return m_i; }
    size_t                  getJ() { return m_j; }


private:
    std::vector<cvf::Vec3d> m_polygon;
    double                  m_value;
    size_t                  m_i;
    size_t                  m_j;

};

