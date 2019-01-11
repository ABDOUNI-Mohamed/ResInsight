#include "RimContourMapViewCollection.h"

#include "RimEclipseContourMapView.h"
#include "RimCase.h"

CAF_PDM_SOURCE_INIT(RimContourMapViewCollection, "Eclipse2dViewCollection");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimContourMapViewCollection::RimContourMapViewCollection()
{
    CAF_PDM_InitObject("Contour Maps", ":/2DMaps16x16.png", "", "");

    CAF_PDM_InitFieldNoDefault(&m_contourMapViews, "EclipseViews", "Contour Maps", ":/CrossSection16x16.png", "", "");
    m_contourMapViews.uiCapability()->setUiTreeHidden(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimContourMapViewCollection::~RimContourMapViewCollection()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RimEclipseContourMapView*> RimContourMapViewCollection::views()
{
    return m_contourMapViews.childObjects();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimContourMapViewCollection::push_back(RimEclipseContourMapView* contourMap)
{
    m_contourMapViews.push_back(contourMap);
}

