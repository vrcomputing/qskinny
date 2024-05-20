/******************************************************************************
 * QSkinny - Copyright (C) The authors
 *           SPDX-License-Identifier: BSD-3-Clause
 *****************************************************************************/

#include "Plot.h"

#include <QskWindow.h>
#include <QskMainView.h>
#include <QskLinearBox.h>
#include <QskPushButton.h>
#include <QskObjectCounter.h>

#include <SkinnyShortcut.h>

#include <QGuiApplication>
#include <QTimer>
#include <QSGSimpleRectNode>
#include <QSGGeometryNode>
#include <QSGMaterialShader>

#include <QtShaderTools/6.7.0/QtShaderTools/rhi/qshaderbaker.h>

#include <optional>
#include <qmath.h>

namespace 
{
    class DataSource : public QObject
    {
        Q_OBJECT
      public:
          explicit DataSource(int msec, QObject* parent = nullptr) : QObject(parent)
          {
              auto* const timer = new QTimer(this);
              timer->setInterval(1000);
              connect(timer, &QTimer::timeout, this, [this](){
                  Q_EMIT dataChanged();
                });
              timer->start();
          }

        Q_SIGNALS:
        void dataChanged();
    };

    template<typename T>
    struct Tag
    {
        using TYPE = T;
    };

    double map(double x, double in_min, double in_max, double out_min, double out_max)
    {
        return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    }

    class GeometryNode : public QSGGeometryNode
    {
      public:
        template<typename Material = QSGFlatColorMaterial>
        GeometryNode(QSGGeometry::DrawingMode mode,int vertexCount = 0, Tag<Material> tag = {})
        {
            auto* const geometry = new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(), vertexCount);
            geometry->setDrawingMode(mode);
            setGeometry(geometry);

            auto* const material = new typename decltype(tag)::TYPE;
            setMaterial(material);
        }
    };
    
    class FlatColorMaterial : public QSGFlatColorMaterial
    {
        QRectF m_rect;
        QRectF m_scale;

      public:
        const QRectF& rect() const { return m_rect; }
        void setRect( QRectF rect ) { m_rect = rect; }

        const QRectF& scale() const { return m_scale; }
        void setScale( QRectF scale ) { m_scale = scale; }

        QSGMaterialShader* createShader(
            QSGRendererInterface::RenderMode renderMode ) const override
        {
            class Shader : public QSGMaterialShader
            {
              public:
                Shader()
                {
                    static const auto* const vertexShader =
                        "#version 440\n"
                        "\n"
                        "layout(location = 0) in vec4 vertexCoord;\n"
                        "\n"
                        "layout(std140, binding = 0) uniform buf {\n"
                        "    mat4 matrix;\n"
                        "    vec4 color;\n"
                        "    vec4 rect;\n"
                        "    vec4 scale;\n"
                        "} ubuf;\n"
                        "\n"
                        "out gl_PerVertex { vec4 gl_Position; };\n"
                        "\n"
                        "void main()\n"
                        "{\n"
                        "    vec4 coord = vertexCoord;\n"
                        "    coord.x = gl_VertexIndex * ubuf.scale.x;\n"
                        "    gl_Position = ubuf.matrix * coord;\n"
                        "}";

                    static const auto* const fragmentShader =
                        "#version 440\n"
                        "\n"
                        "layout(location = 0) out vec4 fragColor;\n"
                        "\n"
                        "layout(std140, binding = 0) uniform buf {\n"
                        "    mat4 matrix;\n"
                        "    vec4 color;\n"
                        "    vec4 rect;\n"
                        "    vec4 scale;\n"
                        "} ubuf;\n"
                        "\n"
                        "void main()\n"
                        "{\n"
                        "    fragColor = ubuf.color;\n"
                        "}";

                    QList< QShaderBaker::GeneratedShader > targets;
                    targets.append( { QShader::SpirvShader, QShaderVersion( 100 ) } );
                    targets.append(
                        { QShader::GlslShader, QShaderVersion( 100, QShaderVersion::GlslEs ) } );
                    targets.append( { QShader::SpirvShader, QShaderVersion( 120 ) } );
                    targets.append( { QShader::HlslShader, QShaderVersion( 50 ) } );
                    targets.append( { QShader::MslShader, QShaderVersion( 12 ) } );

                    QShaderBaker baker;
                    baker.setGeneratedShaderVariants( { QShader::StandardShader } );
                    baker.setGeneratedShaders( targets );

                    {
                        baker.setSourceString( vertexShader, QShader::VertexStage );
                        auto shaders = baker.bake();
                        if ( !shaders.isValid() )
                        {
                            qWarning() << baker.errorMessage();
                        }

                        setShader( QSGMaterialShader::VertexStage, shaders );
                    }

                    {
                        baker.setSourceString( fragmentShader, QShader::FragmentStage );
                        auto shaders = baker.bake();
                        if ( !shaders.isValid() )
                        {
                            qWarning() << baker.errorMessage();
                        }

                        setShader( QSGMaterialShader::FragmentStage, shaders );
                    }

                    // setShaderFileName( QSGMaterialShader::VertexStage,
                    //     QStringLiteral(
                    //         ":/qt-project.org/scenegraph/shaders_ng/flatcolor.vert.qsb" ) );
                    // setShaderFileName( QSGMaterialShader::FragmentStage,
                    //     QStringLiteral(
                    //         ":/qt-project.org/scenegraph/shaders_ng/flatcolor.frag.qsb" ) );
                }

                bool updateUniformData( RenderState& state, QSGMaterial* newMaterial,
                    QSGMaterial* oldMaterial ) override
                {
                    Q_ASSERT( !oldMaterial || newMaterial->type() == oldMaterial->type() );
                    auto* oldMat = static_cast< FlatColorMaterial* >( oldMaterial );
                    auto* newMat = static_cast< FlatColorMaterial* >( newMaterial );
                    bool changed = false;
                    QByteArray* buf = state.uniformData();
                    if ( state.isMatrixDirty() )
                    {
                        const QMatrix4x4 m = state.combinedMatrix();
                        memcpy( buf->data(), m.constData(), 64 );
                        changed = true;
                    }

                    const QColor& c = newMat->color();
                    if ( !oldMat || c != oldMat->color() || state.isOpacityDirty() )
                    {
                        float r, g, b, a;
                        c.getRgbF( &r, &g, &b, &a );
                        const float opacity = state.opacity() * a;
                        QVector4D v( r * opacity, g * opacity, b * opacity, opacity );
                        Q_ASSERT( sizeof( v ) == 16 );
                        memcpy( buf->data() + 64, &v, 16 );

                        QVector4D rect{ ( float ) newMat->rect().x(), ( float ) newMat->rect().x(),
                            ( float ) newMat->rect().x(), ( float ) newMat->rect().x() };
                        std::memcpy( buf->data() + 64 + 16, &rect, 16 );

                        QVector4D scale{ ( float ) newMat->scale().x(), ( float ) newMat->scale().x(),
                            ( float ) newMat->scale().x(), ( float ) newMat->scale().x() };
                        std::memcpy( buf->data() + 64 + 16 + 16, &scale, 16 );

                        changed = true;
                    }

                    return changed;
                }
            };

            return new Shader;
        }

        QSGMaterialType* type() const override
        {
            static QSGMaterialType type;
            return &type;
        }
    };


    [[nodiscard]] auto* createRectanglesNode(const QColor& color, const int count = 1)
    {
        auto* const node = new GeometryNode(QSGGeometry::DrawTriangleStrip, count * 4);        
        static_cast<QSGFlatColorMaterial*>(node->material())->setColor(color);
        return node;
    }

    [[nodiscard]] auto* createLinesNode(const QColor& color, const int count = 1, const float width = 1.0f)
    {
        auto* const node = new GeometryNode(QSGGeometry::DrawLines, count * 2);        
        static_cast<QSGFlatColorMaterial*>(node->material())->setColor(color);
        node->geometry()->setLineWidth( width );
        return node;
    }

    [[nodiscard]] auto* createLineStripNode(const QColor& color, const int count = 1, const float width = 1.0f)
    {
        auto* const node = new GeometryNode(QSGGeometry::DrawLineStrip, count * 2, Tag<FlatColorMaterial>{});
        static_cast<QSGFlatColorMaterial*>(node->material())->setColor(color);
        node->geometry()->setLineWidth( width );
        return node;
    }

    class Stripe : public QSGNode
    {    
    public:
        Stripe(const QColor& color)
        {
            appendChildNode(createLineStripNode(color, 100));
        }

        void update(const QRectF& rect, const QSizeF& windowSize)
        {
            static const auto start = QDateTime::currentMSecsSinceEpoch();
            const auto now = QDateTime::currentMSecsSinceEpoch();

            auto* const lines = static_cast< GeometryNode* >( firstChild() );                       

            auto* const geometry = lines->geometry();
            auto* const line = geometry->vertexDataAsPoint2D();

            const auto scaleX = rect.width() / geometry->vertexCount();
            const auto scaleY = rect.height() / 60;
           
            static_cast< FlatColorMaterial* >( lines->material() )->setRect( { rect } );
            static_cast< FlatColorMaterial* >( lines->material() )->setScale( { scaleX, 0, 0, 0 } );

            auto* const vertices = geometry->vertexDataAsPoint2D();
            const auto y = rand() % 60 * scaleY;
            for ( int i = 0; i < geometry->vertexCount(); ++i ) 
            {
                // vertices[ i ].x = i * scaleX;
                vertices[ i ].y = y;
            }

            geometry->markVertexDataDirty();
            lines->markDirty(QSGNode::DirtyGeometry);
        }
    };

    class View : public QQuickItem
    {    
    public:
        explicit View(QQuickItem* parent = nullptr) : QQuickItem(parent)
        {
            setFlag( QQuickItem::ItemHasContents);

            setAcceptedMouseButtons(Qt::LeftButton);
            setAcceptHoverEvents(true);

            setCursor( Qt::CrossCursor);

            auto* const updater = new QTimer(this);
            updater->setInterval(10);
            connect(updater, &QTimer::timeout, this, [this](){
                update();
            });
            updater->start();
        }

    protected:

        void mousePressEvent(QMouseEvent *event) override
        {
            m_mouseSelectionBegin = event->pos().toPointF();
            m_mouseSelectionEnd = m_mouseSelectionBegin;
            event->accept();
            update();
        }

        void mouseReleaseEvent(QMouseEvent *event) override
        {
            m_mouseSelectionEnd = event->pos().toPointF();
            event->accept();
            update();
        }

        void mouseMoveEvent(QMouseEvent *event) override
        {
        }

        void hoverMoveEvent(QHoverEvent *event) override
        {
            m_mousePos = event->pos();
            event->accept();
            update();
        }

        QSGNode *updatePaintNode(QSGNode* const node, UpdatePaintNodeData *) override
        {            
            QSGSimpleRectNode *n = static_cast<QSGSimpleRectNode *>(node);
            if (!n) 
            {
                n = new QSGSimpleRectNode();
                n->setColor(Qt::lightGray);

                // Axis Lines
                n->appendChildNode(createLinesNode(Qt::black, 2));
                // Horizontal Range Selection
                n->appendChildNode(createRectanglesNode(QColor(0,255,0,128), 1));
                // Mouse Lines
                n->appendChildNode(createLinesNode(Qt::black, 2));
                // Line Strip
                {
                    QColor colors[3] = {Qt::red, Qt::green, Qt::blue};
                    auto* const lineStrips = new QSGNode;
                    {
                        for ( int i = 0; i < 20; ++i )
                        {
                            auto* const transform = new QSGTransformNode;
                            transform->appendChildNode( new Stripe( colors[ i % 3] ) );
                            lineStrips->appendChildNode(transform);
                        }                        
                    }
                    n->appendChildNode( lineStrips );
                }
            }

            n->setRect(boundingRect());

            // Axis Lines
            {
                auto* const lines = static_cast< GeometryNode* >( n->childAtIndex( 0 ) );
                auto* const geometry = lines->geometry();
                auto* const line = geometry->vertexDataAsPoint2D();
                // horizontal
                line[ 0 ].set( boundingRect().left(), boundingRect().center().y() );
                line[ 1 ].set( boundingRect().right(), boundingRect().center().y() );
                // vertical
                line[ 2 ].set( boundingRect().center().x(), boundingRect().top() );
                line[ 3 ].set( boundingRect().center().x(), boundingRect().bottom() );
                geometry->markVertexDataDirty();
                lines->markDirty(QSGNode::DirtyGeometry);
            }
            // Horizontal Selection
            {
                auto* const lines = static_cast< GeometryNode* >( n->childAtIndex( 1 ) );
                auto* const geometry = lines->geometry();
                auto* const line = geometry->vertexDataAsPoint2D();
                // horizontal
                line[ 0 ].set( selection().left(), boundingRect().top() );
                line[ 1 ].set( selection().left(), boundingRect().bottom() );
                line[ 2 ].set( selection().right(), boundingRect().top() );
                line[ 3 ].set( selection().right(), boundingRect().bottom() );
                geometry->markVertexDataDirty();
                lines->markDirty(QSGNode::DirtyGeometry);
            }
            // Mouse Lines
            {
                auto* const lines = static_cast< GeometryNode* >( n->childAtIndex( 2 ) );
                auto* const geometry = lines->geometry();
                auto* const line = geometry->vertexDataAsPoint2D();
                // horizontal
                line[ 0 ].set( boundingRect().left(), m_mousePos.y() );
                line[ 1 ].set( boundingRect().right(), m_mousePos.y() );
                // vertical
                line[ 2 ].set( m_mousePos.x(), boundingRect().top() );
                line[ 3 ].set( m_mousePos.x(), boundingRect().bottom() );
                geometry->markVertexDataDirty();
                lines->markDirty(QSGNode::DirtyGeometry);
            }
            // Line Strip
            {
                auto* const lineStrips = static_cast< QSGNode* >( n->childAtIndex( 3 ) );

                auto rect = boundingRect();
                rect.setHeight(boundingRect().height() / lineStrips->childCount());

                const auto windowSize = window()->size();

                for ( int i = 0; i < lineStrips->childCount(); ++i )
                {
                    auto* const transform = static_cast< QSGTransformNode* >( lineStrips->childAtIndex( i ) );
                    auto* const strip = static_cast< Stripe* >( transform->firstChild() );

                    QTransform t;
                    t.translate( rect.left(), i * rect.height() );
                    transform->setMatrix(t);

                    rect.moveTop(i * rect.height());
                    strip->update(rect, windowSize);
                }
            }

            return n;
        }

        [[nodiscard]] QRectF selection() const
        {
            const auto x1 = qMin(m_mouseSelectionBegin.x(), m_mouseSelectionEnd.x());
            const auto x2 = qMax(m_mouseSelectionBegin.x(), m_mouseSelectionEnd.x());
            const auto y1 = qMin(m_mouseSelectionBegin.y(), m_mouseSelectionEnd.y());
            const auto y2 = qMax(m_mouseSelectionBegin.y(), m_mouseSelectionEnd.y());
            return {
                QPointF{x1,y1},
                QPointF{x2,y2}
            };
        }

    private:
        QPointF m_mousePos;
        QPointF m_mouseSelectionBegin;
        QPointF m_mouseSelectionEnd;
    };
}

namespace
{
    class TestPlot : public Plot
    {
      public:
        TestPlot( QQuickItem* parentItem = nullptr )
            : Plot( parentItem )
        {
            setMargins( 5 );
        }
    };

    class Header : public QskLinearBox
    {
        Q_OBJECT

      public:
        Header( QQuickItem* parent = nullptr )
            : QskLinearBox( Qt::Horizontal, parent )
        {
            initSizePolicy( QskSizePolicy::Ignored, QskSizePolicy::Fixed );

            setPanel( true );
            setPaddingHint( QskBox::Panel, 5 );

            addStretch( 1 );

            auto buttonLeft = new QskPushButton( "<", this );
            buttonLeft->setAutoRepeat( true );
            connect( buttonLeft, &QskPushButton::clicked,
                this, [this] { Q_EMIT shiftClicked( +1 ); } );

            auto buttonRight = new QskPushButton( ">", this );
            buttonRight->setAutoRepeat( true );
            connect( buttonRight, &QskPushButton::clicked,
                this, [this] { Q_EMIT shiftClicked( -1 ); } );

            auto buttonReset = new QskPushButton( "Reset", this );
            connect( buttonReset, &QskPushButton::clicked,
                this, &Header::resetClicked );
        }

      Q_SIGNALS:
        void shiftClicked( int steps );
        void resetClicked();
    };

    class MainView : public QskMainView
    {
      public:
        MainView( QQuickItem* parent = nullptr )
            : QskMainView( parent )
        {
            auto header = new Header( this );
            auto plot1 = new TestPlot();
            auto plot2 = new TestPlot();

            auto* const timer = new QTimer(this);
            timer->setInterval(10);
            timer->start();
            connect(timer, &QTimer::timeout, this, [plot1](){
                QVector< Plot::Sample > samples;
                samples.reserve(1000);
                for(int i = 0; i < 1000; ++i)
                {
                    samples += { (qreal)i, 0, (qreal)(rand() % 10), 10 };
                }
                plot1->setSamples(samples);
            });
            connect(timer, &QTimer::timeout, this, [plot2](){
                QVector< Plot::Sample > samples;
                samples.reserve(1000);
                for(int i = 0; i < 1000; ++i)
                {
                    samples += { (qreal)i, 0, (qreal)(rand() % 10), 10 };
                }
                plot2->setSamples(samples);
            });

            connect( header, &Header::resetClicked,
                plot1, &Plot::resetAxes );

            connect( header, &Header::shiftClicked,
                plot1, &Plot::shiftXAxis );

            setHeader( header );
            setBody( plot1 );
        }
    };
}

int main( int argc, char* argv[] )
{
#ifdef ITEM_STATISTICS
    QskObjectCounter counter( true );
#endif

    QGuiApplication app( argc, argv );

    SkinnyShortcut::enable( SkinnyShortcut::AllShortcuts );

    QskWindow window;
    window.addItem( new View() );
    window.resize( 800, 600 );
    window.show();

    return app.exec();
}

#include "main.moc"
