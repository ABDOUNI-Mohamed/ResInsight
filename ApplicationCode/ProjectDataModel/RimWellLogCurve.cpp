/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "RimWellLogCurve.h"

#include "RiaCurveDataTools.h"
#include "RigWellLogCurveData.h"

#include "RimWellLogPlot.h"
#include "RimWellLogTrack.h"

#include "RiuQwtPlotCurve.h"
#include "RiuQwtPlotWidget.h"

#include "cafPdmUiComboBoxEditor.h"

#include "cvfAssert.h"

#include "qwt_symbol.h"

#include <algorithm>

// NB! Special macro for pure virtual class
CAF_PDM_XML_ABSTRACT_SOURCE_INIT( RimWellLogCurve, "WellLogPlotCurve" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellLogCurve::RimWellLogCurve()
{
    CAF_PDM_InitObject( "WellLogCurve", ":/WellLogCurve16x16.png", "", "" );

    m_qwtPlotCurve->setXAxis( QwtPlot::xTop );
    m_qwtCurveErrorBars->setXAxis( QwtPlot::xTop );
    m_qwtPlotCurve->setYAxis( QwtPlot::yLeft );

    m_curveData = new RigWellLogCurveData;

    m_curveDataXRange = std::make_pair( std::numeric_limits<double>::infinity(), -std::numeric_limits<double>::infinity() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellLogCurve::~RimWellLogCurve()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellLogCurve::xValueRangeInData( double* minimumValue, double* maximumValue ) const
{
    CAF_ASSERT( minimumValue && maximumValue );

    if ( !( minimumValue && maximumValue ) )
    {
        return false;
    }

    if ( m_curveDataXRange.first == -std::numeric_limits<double>::infinity() ||
         m_curveDataXRange.second == std::numeric_limits<double>::infinity() )
    {
        return false;
    }

    *minimumValue = m_curveDataXRange.first;
    *maximumValue = m_curveDataXRange.second;

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellLogCurve::yValueRangeInData( double* minimumValue, double* maximumValue ) const
{
    CAF_ASSERT( minimumValue && maximumValue );

    if ( !( minimumValue && maximumValue ) )
    {
        return false;
    }

    RimWellLogPlot* wellLogPlot = nullptr;
    firstAncestorOrThisOfTypeAsserted( wellLogPlot );
    auto depthType   = wellLogPlot->depthType();
    auto displayUnit = wellLogPlot->depthUnit();

    return m_curveData->calculateDepthRange( depthType, displayUnit, minimumValue, maximumValue );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogCurve::setValuesAndDepths( const std::vector<double>& xValues,
                                          const std::vector<double>& depths,
                                          RiaDefines::DepthTypeEnum  depthType,
                                          double                     rkbDiff,
                                          RiaDefines::DepthUnitType  depthUnit,
                                          bool                       isExtractionCurve,
                                          const QString&             xUnits )
{
    m_curveData->setValuesAndDepths( xValues, depths, depthType, rkbDiff, depthUnit, isExtractionCurve );
    m_curveData->setXUnits( xUnits );
    calculateCurveDataXRange();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogCurve::setValuesAndDepths( const std::vector<double>&                                      xValues,
                                          const std::map<RiaDefines::DepthTypeEnum, std::vector<double>>& depths,
                                          double                                                          rkbDiff,
                                          RiaDefines::DepthUnitType                                       depthUnit,
                                          bool           isExtractionCurve,
                                          const QString& xUnits )
{
    m_curveData->setValuesAndDepths( xValues, depths, rkbDiff, depthUnit, isExtractionCurve );
    m_curveData->setXUnits( xUnits );
    calculateCurveDataXRange();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogCurve::setValuesWithMdAndTVD( const std::vector<double>& xValues,
                                             const std::vector<double>& measuredDepths,
                                             const std::vector<double>& tvdMSL,
                                             double                     rkbDiff,
                                             RiaDefines::DepthUnitType  depthUnit,
                                             bool                       isExtractionCurve,
                                             const QString&             xUnits )
{
    std::map<RiaDefines::DepthTypeEnum, std::vector<double>> depths = {{RiaDefines::MEASURED_DEPTH, measuredDepths},
                                                                       {RiaDefines::TRUE_VERTICAL_DEPTH, tvdMSL}};
    setValuesAndDepths( xValues, depths, rkbDiff, depthUnit, isExtractionCurve, xUnits );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RigWellLogCurveData* RimWellLogCurve::curveData() const
{
    return m_curveData.p();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimWellLogCurve::wellLogChannelName() const
{
    return wellLogChannelUiName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogCurve::updateZoomInParentPlot()
{
    const double eps = 1.0e-8;

    RimWellLogTrack* wellLogTrack;
    firstAncestorOrThisOfType( wellLogTrack );

    if ( wellLogTrack )
    {
        wellLogTrack->setAutoScaleXIfNecessary();

        RimWellLogPlot* wellLogPlot;
        wellLogTrack->firstAncestorOrThisOfType( wellLogPlot );

        if ( wellLogPlot )
        {
            double minPlotDepth, maxPlotDepth;
            wellLogPlot->availableDepthRange( &minPlotDepth, &maxPlotDepth );

            bool updateDepthZoom = false;
            if ( minPlotDepth == std::numeric_limits<double>::infinity() ||
                 maxPlotDepth == -std::numeric_limits<double>::infinity() )
            {
                updateDepthZoom = true;
            }
            else
            {
                double plotRange = std::abs( maxPlotDepth - minPlotDepth );
                double minCurveDepth, maxCurveDepth;
                m_curveData->calculateDepthRange( wellLogPlot->depthType(),
                                                  wellLogPlot->depthUnit(),
                                                  &minCurveDepth,
                                                  &maxCurveDepth );
                updateDepthZoom = minCurveDepth < minPlotDepth - eps * plotRange ||
                                  maxCurveDepth > maxPlotDepth + eps * plotRange;
            }
            if ( updateDepthZoom )
            {
                wellLogPlot->setAutoScaleDepthEnabled( true );
            }
            wellLogPlot->updateZoom();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogCurve::updateLegendsInPlot()
{
    RimWellLogTrack* wellLogTrack;
    firstAncestorOrThisOfType( wellLogTrack );
    if ( wellLogTrack )
    {
        wellLogTrack->updateLegend();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogCurve::setOverrideCurveDataXRange( double minimumValue, double maximumValue )
{
    m_curveDataXRange = std::make_pair( minimumValue, maximumValue );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogCurve::calculateCurveDataXRange()
{
    // Invalidate range first
    m_curveDataXRange = std::make_pair( std::numeric_limits<double>::infinity(), -std::numeric_limits<double>::infinity() );
    for ( double xValue : m_curveData->xValues() )
    {
        if ( RiaCurveDataTools::isValidValue( xValue, false ) )
        {
            m_curveDataXRange.first  = std::min( m_curveDataXRange.first, xValue );
            m_curveDataXRange.second = std::max( m_curveDataXRange.second, xValue );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogCurve::fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                        const QVariant&            oldValue,
                                        const QVariant&            newValue )
{
    RimPlotCurve::fieldChangedByUi( changedField, oldValue, newValue );
    if ( changedField == &m_showCurve )
    {
        updateZoomInParentPlot();
    }
}
