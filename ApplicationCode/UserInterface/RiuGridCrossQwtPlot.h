/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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

#include "RiuInterfaceToViewWindow.h"
#include "RiuPlotAnnotationTool.h"
#include "RiuQwtPlot.h"

#include "cafPdmPointer.h"

#include <QPointer>

#include <memory>

class RimGridCrossPlotCurveSet;
class RiuCvfOverlayItemWidget;
class RiuDraggableOverlayFrame;
class RiuPlotAnnotationTool;
class RimPlotAxisProperties;

namespace caf
{
class TitledOverlayFrame;
}
//==================================================================================================
//
//
//
//==================================================================================================
class RiuGridCrossQwtPlot : public RiuQwtPlot
{
    Q_OBJECT;

public:
    RiuGridCrossQwtPlot(RimViewWindow* ownerViewWindow, QWidget* parent = nullptr);

    void addOrUpdateCurveSetLegend(RimGridCrossPlotCurveSet* curveSetToShowLegendFor);
    void removeCurveSetLegend(RimGridCrossPlotCurveSet* curveSetToShowLegendFor);
    void updateLegendSizesToMatchPlot();
    void updateAnnotationObjects(RimPlotAxisProperties* axisProperties);

protected:
    void updateLayout() override;
    void updateInfoBoxLayout();
    void updateLegendLayout();
    void resizeEvent(QResizeEvent* e) override;
    bool resizeOverlayItemToFitPlot(caf::TitledOverlayFrame* overlayItem);
    void contextMenuEvent(QContextMenuEvent*) override;

private:
    typedef caf::PdmPointer<RimGridCrossPlotCurveSet> CurveSetPtr;
    typedef QPointer<RiuCvfOverlayItemWidget> LegendPtr;
    typedef QPointer<RiuDraggableOverlayFrame> InfoBoxPtr;

    InfoBoxPtr                             m_infoBox;
    std::map<CurveSetPtr, LegendPtr>       m_legendWidgets;
    std::unique_ptr<RiuPlotAnnotationTool> m_annotationTool;
};