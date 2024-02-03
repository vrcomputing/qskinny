#include "QskAlphaSliderSkinlet.h"
#include "QskAlphaSlider.h"

#include "QskSlider.h"
#include <QskBoxBorderMetrics.h>
#include <QskBoxFillNode.h>
#include <QskBoxRenderer.h>
#include <QskBoxShapeMetrics.h>
#include <QskGradient.h>
#include <QskSGNode.h>
#include <QskSliderSkinlet.h>

#include <QSGMaterial>
#include <QSGMaterialShader>
#include <QSGVertexColorMaterial>

namespace
{
    class Material : public QSGMaterial
    {
      public:
        QSGMaterialType* type() const override
        {
            static QSGMaterialType type;
            return &type;
        }

        QSGMaterialShader* createShader() const override;

        QColor m_colorA;
        QColor m_colorB;
        float m_gridSize;
    };

    class Shader : public QSGMaterialShader
    {
      public:
        const char* vertexShader() const
        {
            return "attribute highp vec4 vertex;          \n"
                   "uniform highp mat4 matrix;            \n"
                   "void main() {                         \n"
                   "    gl_Position = matrix * vertex;    \n"
                   "}";
        }

        const char* fragmentShader() const
        {
            return "uniform lowp float opacity;                                         \n"
                   "uniform lowp vec4 colorA;                                           \n"
                   "uniform lowp vec4 colorB;                                           \n"
                   "uniform lowp float size;                                            \n"
                   "                                                                    \n"
                   "vec4 color(in vec4 fragColor, in vec2 fragCoord, in float gridSize) \n"
                   "{                                                                   \n"
                   "    vec2 pos = floor(fragCoord / gridSize);                         \n"
                   "    float mask = mod(pos.x + mod(pos.y, 2.0), 2.0);                 \n"
                   "    return mask * fragColor;                                        \n"
                   "}                                                                   \n"
                   "                                                                    \n"
                   "void main() {                                                       \n"
                   "    vec2 fragCoordA = gl_FragCoord.xy;                              \n"
                   "    vec2 fragCoordB = gl_FragCoord.xy + vec2(size, 0.0);            \n"
                   "    gl_FragColor = color(colorA, fragCoordA, size) +                \n"
                   "                   color(colorB, fragCoordB, size) *                \n"
                   "                   opacity;                                         \n"
                   "}";
        }

        char const* const* attributeNames() const
        {
            static char const* const names[] = { "vertex", 0 };
            return names;
        }

        void initialize()
        {
            QSGMaterialShader::initialize();
            m_id.matrix = program()->uniformLocation( "matrix" );
            m_id.opacity = program()->uniformLocation( "opacity" );
            m_id.color_a = program()->uniformLocation( "colorA" );
            m_id.color_b = program()->uniformLocation( "colorB" );
            m_id.grid_size = program()->uniformLocation( "size" );
        }

        void updateState(
            const RenderState& state, QSGMaterial* newMaterial, QSGMaterial* oldMaterial )
        {
            Q_ASSERT( program()->isLinked() );

            if ( state.isMatrixDirty() )
            {
                program()->setUniformValue( m_id.matrix, state.combinedMatrix() );
            }

            if ( state.isOpacityDirty() )
            {
                program()->setUniformValue( m_id.opacity, state.opacity() );
            }

            const auto* const material = static_cast< const Material* >( newMaterial );
            program()->setUniformValue( m_id.color_a, material->m_colorA );
            program()->setUniformValue( m_id.color_b, material->m_colorB );
            program()->setUniformValue( m_id.grid_size, material->m_gridSize );
        }

      private:
        struct
        {
            int matrix = -1;
            int opacity = -1;
            int color_a = -1;
            int color_b = -1;
            int grid_size = -1;
        } m_id;
    };

    QSGMaterialShader* Material::createShader() const
    {
        return new Shader;
    }

    class GridNode : public QSGGeometryNode
    {
      public:
        GridNode()
            : m_geometry( QSGGeometry::defaultAttributes_Point2D(), 0 )
        {
            setGeometry( &m_geometry );
            setMaterial( &m_material );
            setFlag( QSGGeometryNode::OwnsGeometry, false );
            setFlag( QSGGeometryNode::OwnsMaterial, false );
        }

        void update( const QRectF& rect, const QskBoxShapeMetrics& shape,
            const QskBoxBorderMetrics& border, const QColor& color1, const QColor& color2,
            const float gridSize )
        {
            QskBoxRenderer::renderFillGeometry( rect, shape, border, m_geometry );
            m_material.m_colorA = color1;
            m_material.m_colorB = color2;
            m_material.m_gridSize = gridSize;
            
            // TODO update only if necessary

            m_geometry.markVertexDataDirty();
            markDirty( QSGNode::DirtyMaterial );
            markDirty( QSGNode::DirtyGeometry );
        }

      private:
        QSGGeometry m_geometry;
        Material m_material;
    };
}

using Q = QskAlphaSlider;

QskAlphaSliderSkinlet::QskAlphaSliderSkinlet( QskSkin* skin )
    : Inherited( skin )
{
    setNodeRoles( { PanelRole, GridRole, GrooveRole, FillRole, HandleRole, RippleRole } );
}

QSGNode* QskAlphaSliderSkinlet::updateSubNode(
    const QskSkinnable* skinnable, const quint8 nodeRole, QSGNode* const node ) const
{
    if ( nodeRole == GridRole )
    {
        const auto* const q = static_cast< const Q* >( skinnable );
        const auto rect = q->subControlRect( Q::Groove );
        const auto shape = q->boxShapeHint( Q::Groove ).toAbsolute( rect.size() );
        const auto border = q->boxBorderMetricsHint( Q::Groove );
        const auto gradient = q->gradientHint( Q::Grid );
        const auto gridSize = q->metric( Q::Grid | QskAspect::Size );

        auto* const grid = QskSGNode::ensureNode< GridNode >( node );
        grid->update( rect, shape, border, gradient.startColor(), gradient.endColor(), gridSize );
        return grid;
    }

    return Inherited::updateSubNode( skinnable, nodeRole, node );
}