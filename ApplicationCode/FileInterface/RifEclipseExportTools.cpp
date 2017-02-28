/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017-     Statoil ASA
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

#include "RifEclipseExportTools.h"

#include "RiaApplication.h"
#include "RiaLogging.h"

#include "RigEclipseCaseData.h"
#include "RigFracture.h"
#include "RigFracture.h"
#include "RigMainGrid.h"

#include "RimEclipseCase.h"
#include "RimEclipseResultDefinition.h"
#include "RimEclipseView.h"
#include "RimEclipseWell.h"
#include "RimEllipseFractureTemplate.h"
#include "RimFracture.h"
#include "RimWellPath.h"

#include "cafProgressInfo.h"

#include <QFile>
#include <QString>
#include <QTextStream>



//--------------------------------------------------------------------------------------------------
/// Constructor
//--------------------------------------------------------------------------------------------------
RifEclipseExportTools::RifEclipseExportTools()
{

}


//--------------------------------------------------------------------------------------------------
/// Destructor
//--------------------------------------------------------------------------------------------------
RifEclipseExportTools::~RifEclipseExportTools()
{
  
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifEclipseExportTools::writeFracturesToTextFile(const QString& fileName,  const std::vector< RimFracture*>& fractures, RimEclipseCase* caseToApply)
{
    RiaLogging::info(QString("Computing and writing COMPDAT values to file %1").arg(fileName));

    if (!(unitsMatchCaseAndFractures(caseToApply, fractures)))
    {
        RiaLogging::error(QString("ERROR: The case selected and relevant fractures does not have consistent unit system"));
        return false;
    }

    RiaApplication* app = RiaApplication::instance();
    RimView* activeView = RiaApplication::instance()->activeReservoirView();
    if (!activeView) return false;
    RimEclipseView* activeRiv = dynamic_cast<RimEclipseView*>(activeView);
    if (!activeRiv) return false;

    const RigMainGrid* mainGrid = activeRiv->mainGrid();
    if (!mainGrid) return false;


    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        return false;
    }

    caf::ProgressInfo pi(fractures.size(), QString("Writing data to file %1").arg(fileName));

    RimEclipseWell* simWell = nullptr;
    RimWellPath* wellPath = nullptr;

    size_t progress =0;
    std::vector<size_t> ijk;

    QTextStream out(&file);
    out << "\n";
    out << "-- Exported from ResInsight" << "\n\n";

    printBackgroundDataHeaderLine(out);


    RiaLogging::debug(QString("Writing intermediate results from COMPDAT calculation"));

    for (RimFracture* fracture : fractures)
    {
        fracture->computeTransmissibility(caseToApply);
        std::vector<RigFractureData> fracDataVector = fracture->attachedRigFracture()->fractureData();

        for (RigFractureData fracData : fracDataVector)
        {
            printBackgroundData(out, wellPath, simWell, fracture, mainGrid, fracData);
        }
    }

    out << "\n";

    out << qSetFieldWidth(7) << "COMPDAT" << "\n" << right << qSetFieldWidth(8);
    for (RimFracture* fracture : fractures)
    {
        RiaLogging::debug(QString("Writing COMPDAT values for fracture %1").arg(fracture->name()));
        fracture->computeTransmissibility(caseToApply);
        std::vector<RigFractureData> fracDataVector = fracture->attachedRigFracture()->fractureData();

        for (RigFractureData fracData : fracDataVector)
        {
            if (fracData.transmissibility > 0)
            {
            printCOMPDATvalues(out, fracData, fracture, wellPath, simWell, mainGrid);
            }
        }
        
        //TODO: If same cell is used for multiple fractures, the sum of contributions should be added to table. 

        progress++;
        pi.setProgress(progress);
    }

    out << "/ \n";

    RiaLogging::info(QString("Competed writing COMPDAT data to file %1").arg(fileName));
    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RifEclipseExportTools::unitsMatchCaseAndFractures(RimEclipseCase* caseToApply, const std::vector<RimFracture *>& fractures)
{
    bool unitsMatch = true;
    RigEclipseCaseData::UnitsType caseUnit = caseToApply->reservoirData()->unitsType();
    for (RimFracture* fracture : fractures)
    {
        if (fracture->attachedFractureDefinition())
        {
            if ((fracture->attachedFractureDefinition()->fractureTemplateUnit) == RimDefines::UNITS_METRIC)
            {
                if (!(caseUnit == RigEclipseCaseData::UNITS_METRIC))
                {
                    unitsMatch = false;
                }
            }
            else if ((fracture->attachedFractureDefinition()->fractureTemplateUnit) == RimDefines::UNITS_FIELD)
            {
                if (!(caseUnit == RigEclipseCaseData::UNITS_FIELD))
                {
                    unitsMatch = false;
                }
            }
        }
    }
    return unitsMatch;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifEclipseExportTools::printCOMPDATvalues(QTextStream & out, RigFractureData &fracData, RimFracture* fracture, RimWellPath* wellPath, RimEclipseWell* simWell, const RigMainGrid* mainGrid)
{
    out << qSetFieldWidth(8);
    if (fracData.transmissibility == cvf::UNDEFINED_DOUBLE || !(fracture->attachedFractureDefinition())) out << "--"; //Commenting out line in output file

    wellPath, simWell = nullptr;
    fracture->firstAncestorOrThisOfType(simWell);
    if (simWell) out << simWell->name;    // 1. Well name 
    fracture->firstAncestorOrThisOfType(wellPath);
    if (wellPath) out << wellPath->name;  // 1. Well name 

    out << qSetFieldWidth(5);

    size_t i, j, k;
    mainGrid->ijkFromCellIndex(fracData.reservoirCellIndex, &i, &j, &k);
    out << i + 1;          // 2. I location grid block, adding 1 to go to eclipse 1-based grid definition
    out << j + 1;          // 3. J location grid block, adding 1 to go to eclipse 1-based grid definition
    out << k + 1;          // 4. K location of upper connecting grid block, adding 1 to go to eclipse 1-based grid definition
    out << k + 1;          // 5. K location of lower connecting grid block, adding 1 to go to eclipse 1-based grid definition

    out << "2* ";         // Default value for 
                         //6. Open / Shut flag of connection
                         // 7. Saturation table number for connection rel perm. Default value

    out << qSetFieldWidth(12);
    // 8. Transmissibility 
    if (fracData.transmissibility != cvf::UNDEFINED_DOUBLE) out << QString::number(fracData.transmissibility, 'e', 4);
    else out << "UNDEF";

    out << qSetFieldWidth(4);
    out << "2* ";         // Default value for 
                         // 9. Well bore diameter. Set to default
                         // 10. Effective Kh (perm times width)

    if (fracture->attachedFractureDefinition())
    {
        out << fracture->attachedFractureDefinition()->skinFactor;    // 11. Skin factor
    }
    else //If no attached fracture definition these parameters are set to UNDEF
    {
        out << "UNDEF";
    }

    out << "/";
    out << " " << fracture->name(); //Fracture name as comment
    out << "\n"; // Terminating entry

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifEclipseExportTools::printBackgroundDataHeaderLine(QTextStream & out)
{
    out << "-- Background data for calculation" << "\n\n";


    //Write header line
    out << qSetFieldWidth(4);
    out << "--";
    out << qSetFieldWidth(12);
    out << "Well ";

    out << qSetFieldWidth(16);
    out << "Fracture ";

    out << qSetFieldWidth(5);
    out << "i";
    out << "j";
    out << "k";

    out << qSetFieldWidth(12);
    out << "Ax";
    out << "Ay";
    out << "Az";
    out << "TotArea";

    out << "skinfac";
    out << "FracLen";

    out << qSetFieldWidth(10);
    out << "DX";
    out << "DY";
    out << "DZ";

    out << qSetFieldWidth(12);
    out << "PermX";
    out << "PermY";
    out << "PermZ";

    out << qSetFieldWidth(8);
    out << "NTG";

    out << qSetFieldWidth(12);
    out << "T_x";
    out << "T_y";
    out << "T_z";

    out << qSetFieldWidth(15);
    out << "Transm";

    out << qSetFieldWidth(20);
    out << "Status";

    out << "\n";
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RifEclipseExportTools::printBackgroundData(QTextStream & out, RimWellPath* wellPath, RimEclipseWell* simWell, RimFracture* fracture, const RigMainGrid* mainGrid, RigFractureData &fracData)
{
    out << qSetFieldWidth(4);
    out << "-- ";

    out << qSetFieldWidth(12);
    wellPath, simWell = nullptr;
    fracture->firstAncestorOrThisOfType(simWell);
    if (simWell) out << simWell->name + " " ;    // 1. Well name 
    fracture->firstAncestorOrThisOfType(wellPath);
    if (wellPath) out << wellPath->name + " ";  // 1. Well name 

    out << qSetFieldWidth(16);
    out << fracture->name().left(15) + " ";


    out << qSetFieldWidth(5);
    size_t i, j, k;
    mainGrid->ijkFromCellIndex(fracData.reservoirCellIndex, &i, &j, &k);
    out << i + 1;          // 2. I location grid block, adding 1 to go to eclipse 1-based grid definition
    out << j + 1;          // 3. J location grid block, adding 1 to go to eclipse 1-based grid definition
    out << k + 1;          // 4. K location of upper connecting grid block, adding 1 to go to eclipse 1-based grid definition

    out << qSetFieldWidth(12);
    //Use f for float, e for exponent float and g for best choice of these two. 
    out << QString::number(fracData.projectedAreas.x(), 'g', 4);
    out << QString::number(fracData.projectedAreas.y(), 'g', 4);
    out << QString::number(fracData.projectedAreas.z(), 'g', 4);
    out << QString::number(fracData.totalArea, 'g', 4);

    out << QString::number(fracData.skinFactor, 'f', 2);
    out << QString::number(fracData.fractureLenght, 'g', 3);

    out << qSetFieldWidth(10);
    out << QString::number(fracData.cellSizes.x(), 'f', 2);
    out << QString::number(fracData.cellSizes.y(), 'f', 2);
    out << QString::number(fracData.cellSizes.z(), 'f', 2);

    out << qSetFieldWidth(12);
    out << QString::number(fracData.permeabilities.x(), 'e', 3);
    out << QString::number(fracData.permeabilities.y(), 'e', 3);
    out << QString::number(fracData.permeabilities.z(), 'e', 3);

    out << qSetFieldWidth(8);
    out << QString::number(fracData.NTG, 'f', 2);

    out << qSetFieldWidth(12);
    out << QString::number(fracData.transmissibilities.x(), 'e', 3);
    out << QString::number(fracData.transmissibilities.y(), 'e', 3);
    out << QString::number(fracData.transmissibilities.z(), 'e', 3);

    out << qSetFieldWidth(15);
    out << QString::number(fracData.transmissibility, 'e', 3);

    if (!fracData.cellIsActive)
    {
        out << qSetFieldWidth(20);
        out << " INACTIVE CELL ";
    }

    else if (fracData.cellIsActive && fracData.transmissibility > 0)
    {
        out << qSetFieldWidth(20);
        out << " ACTIVE CELL ";
    }
    else
    {
        out << qSetFieldWidth(20);
        out << " INVALID DATA ";
    }

    out << "\n";

}
