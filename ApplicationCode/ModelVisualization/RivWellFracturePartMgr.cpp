/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

#include "RivWellFracturePartMgr.h"

#include "RiaApplication.h"

#include "RimEclipseView.h"
#include "RimEclipseWell.h"
#include "RimFracture.h"
#include "RimFractureTemplate.h"
#include "RimLegendConfig.h"
#include "RimStimPlanColors.h"
#include "RimStimPlanFractureTemplate.h"

#include "RivPartPriority.h"
#include "RivObjectSourceInfo.h"

#include "cafDisplayCoordTransform.h"
#include "cafEffectGenerator.h"

#include "cvfDrawableGeo.h"
#include "cvfModelBasicList.h"
#include "cvfPart.h"
#include "cvfPrimitiveSet.h"
#include "cvfPrimitiveSetIndexedUInt.h"
#include "cvfScalarMapperContinuousLinear.h"
#include "cvfRenderStateDepth.h"
#include "RivFaultGeometryGenerator.h"


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivWellFracturePartMgr::RivWellFracturePartMgr(RimFracture* fracture)
    : m_rimFracture(fracture)
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RivWellFracturePartMgr::~RivWellFracturePartMgr()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivWellFracturePartMgr::updatePartGeometry(caf::DisplayCoordTransform* displayCoordTransform)
{
    if (m_part.notNull()) return;
    if (!displayCoordTransform) return;

    if (m_rimFracture)
    {
        if (!m_rimFracture->hasValidGeometry())
        {
            m_rimFracture->computeGeometry();
        }

        if (!m_rimFracture->hasValidGeometry()) return;

        const std::vector<cvf::Vec3f>& nodeCoords = m_rimFracture->nodeCoords();
        const std::vector<cvf::uint>& triangleIndices = m_rimFracture->triangleIndices();
        std::vector<cvf::Vec3f> displayCoords;

        for (size_t i = 0; i < nodeCoords.size(); i++)
        {
            cvf::Vec3d nodeCoordsDouble = static_cast<cvf::Vec3d>(nodeCoords[i]);
            cvf::Vec3d displayCoordsDouble = displayCoordTransform->transformToDisplayCoord(nodeCoordsDouble);
            displayCoords.push_back(static_cast<cvf::Vec3f>(displayCoordsDouble));
        }

        cvf::ref<cvf::DrawableGeo> geo = createGeo(triangleIndices, displayCoords);

        m_part = new cvf::Part;
        m_part->setDrawable(geo.p());

        cvf::Color4f fractureColor = cvf::Color4f(cvf::Color3f(cvf::Color3::BROWN));

        RimEclipseView* activeView = dynamic_cast<RimEclipseView*>(RiaApplication::instance()->activeReservoirView());
        if (activeView)
        {
            fractureColor = cvf::Color4f(activeView->stimPlanColors->defaultColor());
        }

        caf::SurfaceEffectGenerator surfaceGen(fractureColor, caf::PO_1);
        cvf::ref<cvf::Effect> eff = surfaceGen.generateCachedEffect();
        m_part->setEffect(eff.p());

        m_part->setSourceInfo(new RivObjectSourceInfo(m_rimFracture));
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivWellFracturePartMgr::updatePartGeometryTexture(caf::DisplayCoordTransform* displayCoordTransform)
{
    if (m_part.notNull()) return;
    if (!displayCoordTransform) return;

    if (m_rimFracture)
    {
        if (!m_rimFracture->hasValidGeometry())
        {
            m_rimFracture->computeGeometry();
        }

        if (!m_rimFracture->hasValidGeometry()) return;

        RimLegendConfig* legendConfig = nullptr;
        RimEclipseView* activeView = dynamic_cast<RimEclipseView*>(RiaApplication::instance()->activeReservoirView());
        if (activeView && activeView->stimPlanColors())
        {
            if (activeView->stimPlanColors()->isChecked())
            {
                legendConfig = activeView->stimPlanColors()->activeLegend();
            }
        }

        // Note : If no legend is found, draw geo using a single color

        RimFractureTemplate * fracTemplate = m_rimFracture->attachedFractureDefinition();
        RimStimPlanFractureTemplate* stimPlanFracTemplate;

        if (dynamic_cast<RimStimPlanFractureTemplate*>(fracTemplate))
        {
            stimPlanFracTemplate = dynamic_cast<RimStimPlanFractureTemplate*>(fracTemplate);
        }
        else return;

        int timeStepIndex = m_rimFracture->stimPlanTimeIndexToPlot;
        std::vector<std::vector<double> > dataToPlot = stimPlanFracTemplate->getDataAtTimeIndex(activeView->stimPlanColors->resultName(), activeView->stimPlanColors->unit(), timeStepIndex);

        const std::vector<cvf::Vec3f>& nodeCoords = m_rimFracture->nodeCoords();
        const std::vector<cvf::uint>& triangleIndices = m_rimFracture->triangleIndices();
        std::vector<cvf::Vec3f> displayCoords;

        for (size_t i = 0; i < nodeCoords.size(); i++)
        {
            cvf::Vec3d nodeCoordsDouble = static_cast<cvf::Vec3d>(nodeCoords[i]);
            cvf::Vec3d displayCoordsDouble = displayCoordTransform->transformToDisplayCoord(nodeCoordsDouble);
            displayCoords.push_back(static_cast<cvf::Vec3f>(displayCoordsDouble));
        }

        cvf::ref<cvf::DrawableGeo> geo = createGeo(triangleIndices, displayCoords);

        m_part = new cvf::Part;
        m_part->setDrawable(geo.p());

        generateFractureOutlinePolygonPart(displayCoordTransform);
        generateStimPlanMeshPart(displayCoordTransform);

        float opacityLevel = activeView->stimPlanColors->opacityLevel();
        if (legendConfig)
        {
            cvf::ScalarMapper* scalarMapper =  legendConfig->scalarMapper();

            cvf::ref<cvf::Vec2fArray> textureCoords = new cvf::Vec2fArray;
            textureCoords->resize(nodeCoords.size());

            int i = 0;
            for (std::vector<double> depthData : dataToPlot)
            {
                std::vector<double> mirroredValuesAtDepth = mirrorDataAtSingleDepth(depthData);
                for (double gridXdata : mirroredValuesAtDepth)
                {
                    cvf::Vec2f texCoord = scalarMapper->mapToTextureCoord(gridXdata);
                    
                    if (gridXdata > 1e-7)
                    {
                        texCoord[1] = 0; // Set the Y texture coordinate to the opaque line in the texture
                    }
                    textureCoords->set(i, texCoord);


                    i++;
                }
            }
     
            geo->setTextureCoordArray(textureCoords.p());

            caf::ScalarMapperEffectGenerator effGen(scalarMapper, caf::PO_1);

            effGen.setOpacityLevel(0.5);
            effGen.discardTransparentFragments(true);

            if (activeView && activeView->isLightingDisabled())
            {
                effGen.disableLighting(true);
            }

            cvf::ref<cvf::Effect> eff = effGen.generateCachedEffect();

            m_part->setEffect(eff.p());
        }
        else
        {
            cvf::Color4f fractureColor = cvf::Color4f(activeView->stimPlanColors->defaultColor(), 1.0);

            caf::SurfaceEffectGenerator surfaceGen(fractureColor, caf::PO_1);
            cvf::ref<cvf::Effect> eff = surfaceGen.generateCachedEffect();
            m_part->setEffect(eff.p());
        }

        m_part->setPriority(RivPartPriority::PartType::Transparent);
        m_part->setSourceInfo(new RivObjectSourceInfo(m_rimFracture));
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivWellFracturePartMgr::generateFractureOutlinePolygonPart(caf::DisplayCoordTransform* displayCoordTransform)
{
    cvf::ref<cvf::DrawableGeo> polygonGeo = createPolygonDrawable(displayCoordTransform);

    m_polygonPart = new cvf::Part;
    m_polygonPart->setDrawable(polygonGeo.p());

    m_polygonPart->updateBoundingBox();
    m_polygonPart->setPriority(RivPartPriority::PartType::TransparentMeshLines);

    caf::MeshEffectGenerator lineEffGen(cvf::Color3::MAGENTA);
    lineEffGen.setLineWidth(3.0f);
    cvf::ref<cvf::Effect> eff = lineEffGen.generateCachedEffect();

    m_polygonPart->setEffect(eff.p());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivWellFracturePartMgr::generateStimPlanMeshPart(caf::DisplayCoordTransform* displayCoordTransform)
{

    if (!m_rimFracture->attachedFractureDefinition()) return;

    RimStimPlanFractureTemplate* stimPlanFracTemplate = dynamic_cast<RimStimPlanFractureTemplate*>(m_rimFracture->attachedFractureDefinition());
    if (!stimPlanFracTemplate) return;

    cvf::ref<cvf::DrawableGeo> stimPlanMeshGeo = createStimPlanMeshDrawable(stimPlanFracTemplate, displayCoordTransform);


    //  From cvf::ref<cvf::DrawableGeo> RivFaultGeometryGenerator::createMeshDrawable()



    m_StimPlanMeshPart = new cvf::Part;
    m_StimPlanMeshPart->setDrawable(stimPlanMeshGeo.p());

    m_StimPlanMeshPart->updateBoundingBox();
    m_StimPlanMeshPart->setPriority(RivPartPriority::PartType::TransparentMeshLines);

    caf::MeshEffectGenerator lineEffGen(cvf::Color3::BLACK);
    lineEffGen.setLineWidth(1.0f);
    cvf::ref<cvf::Effect> eff = lineEffGen.generateCachedEffect();

    m_StimPlanMeshPart->setEffect(eff.p());

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::DrawableGeo> RivWellFracturePartMgr::createStimPlanMeshDrawable(RimStimPlanFractureTemplate* stimPlanFracTemplate, caf::DisplayCoordTransform* displayCoordTransform)
{
    std::vector<double> depthCoordsAtNodes = stimPlanFracTemplate->adjustedDepthCoordsAroundWellPathPosition();
    std::vector<double> xCoordsAtNodes = stimPlanFracTemplate->getNegAndPosXcoords();

    //To show lines in between nodes instead of between nodes
    std::vector<double> xCoords;
    for (int i = 0; i < xCoordsAtNodes.size() -1; i++) xCoords.push_back((xCoordsAtNodes[i] + xCoordsAtNodes[i + 1]) / 1);
    std::vector<double> depthCoords;
    for (int i = 0; i < depthCoordsAtNodes.size() - 1; i++) depthCoords.push_back((depthCoordsAtNodes[i] + depthCoordsAtNodes[i + 1]) / 1);

    std::vector<cvf::Vec3f> stimPlanMeshVertices;
    for (int i = 0; i < xCoords.size() - 1; i++)
    {
        for (int j = 0; j < depthCoords.size() - 1; j++)
        {
            if (stimPlanCellTouchesPolygon(xCoords[i], xCoords[i + 1], depthCoords[j], depthCoords[j + 1]))
            {
                stimPlanMeshVertices.push_back(cvf::Vec3f(static_cast<float>(xCoords[i]), static_cast<float>(depthCoords[j]), 0.0f));
                stimPlanMeshVertices.push_back(cvf::Vec3f(static_cast<float>(xCoords[i + 1]), static_cast<float>(depthCoords[j]), 0.0f));
                stimPlanMeshVertices.push_back(cvf::Vec3f(static_cast<float>(xCoords[i + 1]), static_cast<float>(depthCoords[j + 1]), 0.0f));
                stimPlanMeshVertices.push_back(cvf::Vec3f(static_cast<float>(xCoords[i]), static_cast<float>(depthCoords[j + 1]), 0.0f));
            }
        }
    }

    cvf::Mat4f m = m_rimFracture->transformMatrix();
    std::vector<cvf::Vec3f> stimPlanMeshVerticesDisplayCoords = transfromToFractureDisplayCoords(stimPlanMeshVertices, m, displayCoordTransform);

    cvf::Vec3fArray* stimPlanMeshVertexList;
    stimPlanMeshVertexList = new cvf::Vec3fArray;
    stimPlanMeshVertexList->assign(stimPlanMeshVerticesDisplayCoords);

    cvf::ref<cvf::DrawableGeo> stimPlanMeshGeo = new cvf::DrawableGeo;
    stimPlanMeshGeo->setVertexArray(stimPlanMeshVertexList);
    cvf::ref<cvf::UIntArray> indices = RivFaultGeometryGenerator::lineIndicesFromQuadVertexArray(stimPlanMeshVertexList);
    cvf::ref<cvf::PrimitiveSetIndexedUInt> prim = new cvf::PrimitiveSetIndexedUInt(cvf::PT_LINES);
    prim->setIndices(indices.p());

    stimPlanMeshGeo->addPrimitiveSet(prim.p());

    return stimPlanMeshGeo;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::DrawableGeo> RivWellFracturePartMgr::createPolygonDrawable(caf::DisplayCoordTransform* displayCoordTransform)
{

    std::vector<cvf::Vec3f> polygon = m_rimFracture->attachedFractureDefinition()->fracturePolygon(m_rimFracture->fractureUnit);

    cvf::Mat4f m = m_rimFracture->transformMatrix();
    std::vector<cvf::Vec3f> polygonDisplayCoords = transfromToFractureDisplayCoords(polygon, m, displayCoordTransform);


    std::vector<cvf::uint> lineIndices;
    std::vector<cvf::Vec3f> vertices;

    for (size_t i = 0; i < polygonDisplayCoords.size(); ++i)
    {
        vertices.push_back(cvf::Vec3f(polygonDisplayCoords[i]));
        if (i < polygonDisplayCoords.size() - 1)
        {
            lineIndices.push_back(static_cast<cvf::uint>(i));
            lineIndices.push_back(static_cast<cvf::uint>(i + 1));
        }
    }

    if (vertices.size() == 0) return NULL;

    cvf::ref<cvf::Vec3fArray> vx = new cvf::Vec3fArray;
    vx->assign(vertices);
    cvf::ref<cvf::UIntArray> idxes = new cvf::UIntArray;
    idxes->assign(lineIndices);

    cvf::ref<cvf::PrimitiveSetIndexedUInt> prim = new cvf::PrimitiveSetIndexedUInt(cvf::PT_LINES);
    prim->setIndices(idxes.p());

    cvf::ref<cvf::DrawableGeo> polygonGeo = new cvf::DrawableGeo;
    polygonGeo->setVertexArray(vx.p());
    polygonGeo->addPrimitiveSet(prim.p());

    return polygonGeo;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<cvf::Vec3f> RivWellFracturePartMgr::transfromToFractureDisplayCoords(std::vector<cvf::Vec3f> coordinatesVector, cvf::Mat4f m, caf::DisplayCoordTransform* displayCoordTransform)
{
    std::vector<cvf::Vec3f> polygonInDisplayCoords;
    for (cvf::Vec3f v : coordinatesVector)
    {
        v.transformPoint(m);
        cvf::Vec3d nodeCoordsDouble = static_cast<cvf::Vec3d>(v);
        cvf::Vec3d displayCoordsDouble = displayCoordTransform->transformToDisplayCoord(nodeCoordsDouble);
        polygonInDisplayCoords.push_back(static_cast<cvf::Vec3f>(displayCoordsDouble));

    }
    return polygonInDisplayCoords;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<double> RivWellFracturePartMgr::mirrorDataAtSingleDepth(std::vector<double> depthData)
{
    std::vector<double> mirroredValuesAtGivenDepth;
    mirroredValuesAtGivenDepth.push_back(depthData[0]);
    for (int i = 1; i < (depthData.size()); i++) //starting at 1 since we don't want center value twice
    {
        double valueAtGivenX = depthData[i];
        mirroredValuesAtGivenDepth.insert(mirroredValuesAtGivenDepth.begin(), valueAtGivenX);
        mirroredValuesAtGivenDepth.push_back(valueAtGivenX);
    }

    return mirroredValuesAtGivenDepth;
}



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivWellFracturePartMgr::appendGeometryPartsToModel(cvf::ModelBasicList* model, caf::DisplayCoordTransform* displayCoordTransform)
{
    clearGeometryCache();

    if (!m_rimFracture->isChecked()) return;

    if (m_part.isNull())
    {
        if (m_rimFracture->attachedFractureDefinition())
        {
            if (dynamic_cast<RimStimPlanFractureTemplate*>(m_rimFracture->attachedFractureDefinition()))
            {
                updatePartGeometryTexture(displayCoordTransform);

                RimStimPlanFractureTemplate* stimPlanFracTemplate = dynamic_cast<RimStimPlanFractureTemplate*>(m_rimFracture->attachedFractureDefinition());
                if (stimPlanFracTemplate->showStimPlanMesh() && m_StimPlanMeshPart.notNull())
                {
                    model->addPart(m_StimPlanMeshPart.p());
                }


            }
            else
            {
                updatePartGeometry(displayCoordTransform);
            }
        }
    }

    if (m_part.notNull())
    {
        model->addPart(m_part.p());
    }
    if (m_rimFracture->showPolygonFractureOutline() && m_polygonPart.notNull())
    {
        model->addPart(m_polygonPart.p());
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RivWellFracturePartMgr::clearGeometryCache()
{
    m_part = nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
cvf::ref<cvf::DrawableGeo> RivWellFracturePartMgr::createGeo(const std::vector<cvf::uint>& triangleIndices, const std::vector<cvf::Vec3f>& nodeCoords)
{
    cvf::ref<cvf::DrawableGeo> geo = new cvf::DrawableGeo;

    cvf::ref<cvf::UIntArray> indices = new cvf::UIntArray(triangleIndices);
    cvf::ref<cvf::Vec3fArray> vertices = new cvf::Vec3fArray(nodeCoords);

    geo->setVertexArray(vertices.p());
    geo->addPrimitiveSet(new cvf::PrimitiveSetIndexedUInt(cvf::PT_TRIANGLES, indices.p()));
    geo->computeNormals();

    return geo;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RivWellFracturePartMgr::stimPlanCellTouchesPolygon(double xMin, double xMax, double yMin, double yMax)
{
    std::vector<cvf::Vec3f> polygon = m_rimFracture->attachedFractureDefinition()->fracturePolygon(m_rimFracture->fractureUnit);

    for (cvf::Vec3f v : polygon)
    {
        if (v.x() > xMin && v.x() < xMax)
        {
            if (v.y() > yMin && v.y() < yMax)
            {
                return true;
            }
        }
    }

    return false;
}

