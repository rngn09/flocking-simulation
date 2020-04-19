#include <QMouseEvent>
#include <QGuiApplication>

#include "NGLScene.h"
#include <ngl/Camera.h>
#include <ngl/Light.h>
#include <ngl/Material.h>
#include <ngl/NGLInit.h>
#include <ngl/VAOPrimitives.h>
#include <ngl/ShaderLib.h>

#include "Crowd.h"
#include "Agent.h"
#include "Node.h"
#include "Grid.h"

//---------------------------------------------------------------------------------
/// @brief the increment for x/y translation with mouse movement
const static float INCREMENT=0.01f;

//---------------------------------------------------------------------------------
/// @brief the increment for the wheel zoom
const static float ZOOM=0.1f;

NGLScene::NGLScene()
{
  // re-size the widget to that of the parent (in that case the GLFrame passed in on construction)
  m_rotate=false;
  // mouse rotation values set to 0
  m_spinXFace=135.0f;
  m_spinYFace=0.0f;
  setTitle("Village Simulation");
}

NGLScene::~NGLScene()
{
  std::cout<<"Shutting down NGL, removing VAO's and Shaders\n";
}

//---------------------------------------------------------------------------------

void NGLScene::resizeGL(QResizeEvent *_event)
{
  m_width=_event->size().width()*devicePixelRatio();
  m_height=_event->size().height()*devicePixelRatio();
  // now set the camera size values as the screen size has changed
  m_cam.setShape(45.0f,(float)width()/height(),0.05f,350.0f);
}

void NGLScene::resizeGL(int _w , int _h)
{
  m_cam.setShape(45.0f,(float)_w/_h,0.05f,350.0f);
  m_width=_w*devicePixelRatio();
  m_height=_h*devicePixelRatio();
}

//---------------------------------------------------------------------------------

void NGLScene::initializeGL()
{
  // we must call that first before any other GL commands to load and link the
  // gl commands from the lib, if that is not done program will crash
  ngl::NGLInit::instance();
  glClearColor(0.4f, 0.4f, 0.4f, 1.0f);			   // Grey Background
  // enable depth testing for drawing
  glEnable(GL_DEPTH_TEST);
  // enable multisampling for smoother drawing
  glEnable(GL_MULTISAMPLE);
   // now to load the shader and set the values
  // grab an instance of shader manager
  ngl::ShaderLib *shader=ngl::ShaderLib::instance();
  // we are creating a shader called Phong
  shader->createShaderProgram("Phong");
  // now we are going to create empty shaders for Frag and Vert
  shader->attachShader("PhongVertex",ngl::ShaderType::VERTEX);
  shader->attachShader("PhongFragment",ngl::ShaderType::FRAGMENT);
  // attach the source
  shader->loadShaderSource("PhongVertex","shaders/PhongVertex.glsl");
  shader->loadShaderSource("PhongFragment","shaders/PhongFragment.glsl");
  // compile the shaders
  shader->compileShader("PhongVertex");
  shader->compileShader("PhongFragment");
  // add them to the program
  shader->attachShaderToProgram("Phong","PhongVertex");
  shader->attachShaderToProgram("Phong","PhongFragment");
  // now bind the shader attributes for most NGL primitives we use the following
  // layout attribute 0 is the vertex data (x,y,z)
  shader->bindAttribute("Phong",0,"inVert");
  // attribute 1 is the UV data u,v (if present)
  shader->bindAttribute("Phong",1,"inUV");
  // attribute 2 are the normals x,y,z
  shader->bindAttribute("Phong",2,"inNormal");

  // now we have associated that data we can link the shader
  shader->linkProgramObject("Phong");
  // and make it active ready to load values
  (*shader)["Phong"]->use();
  // the shader will use the currently active material and light0 so set them
  ngl::Material m(ngl::STDMAT::GOLD);
  // load our material values to the shader into the structure material (see Vertex shader)
  m.loadToShader("material");
  // Now we will create a basic Camera from the graphics library
  // This is a static camera so it only needs to be set once
  // First create Values for the camera position
  ngl::Vec3 from(60,40,60);
  ngl::Vec3 to(34,0,34);
  ngl::Vec3 up(0,1,0);
  // now load to our new camera
  m_cam.set(from,to,up);
  // set the shape using FOV 45 Aspect Ratio based on Width and Height
  // The final two are near and far clipping planes of 0.5 and 10
  m_cam.setShape(45.0f,(float)720.0/576.0f,0.05f,350.0f);
  shader->setUniform("viewerPos",m_cam.getEye().toVec3());
  // now create our light that is done after the camera so we can pass the
  // transpose of the projection matrix to the light to do correct eye space
  // transformations
  ngl::Mat4 iv=m_cam.getViewMatrix();
  iv.transpose();
  ngl::Light light(ngl::Vec3(200,200,200),ngl::Colour(1,1,1,1),ngl::Colour(1,1,1,1),ngl::LightModes::POINTLIGHT );
  light.setTransform(iv);
  // load these values to the shader as well
  light.loadToShader("light");
  // as re-size is not explicitly called we need to do that.
  // set the viewport for openGL we need to take into account retina display
  glViewport(0,0,width(),height());
  m_agentTimer=startTimer(20);

  // set grid size
  m_gridSize=50;

  // create agents and grid
  ngl::VAOPrimitives *prim=ngl::VAOPrimitives::instance();
  prim->createSphere("sphere",0.5,10);
  prim->createLineGrid("Grid",m_gridSize,m_gridSize,m_gridSize);

  m_status.set(1,1,1);

/*********************************************************
  There is 4 versions of scenarios:
  - option 1: maze               - number of agent: 2
  - option 2: towards collision  - number of agent: 6
  - option 3: glancing collision - number of agent: 4
  - option 4: bottleneck         - number of agent: 5
**********************************************************/
  scenario(1);

}


//---------------------------------------------------------------------------------


void NGLScene::scenario(int _option)
{
  if ( _option == 1 )
  {
    // create crowd(pos, numAgent, gridSize, status)
    m_crowd.reset(new Crowd(ngl::Vec3(0,-5,0),2,m_gridSize,&m_status));

    // create maze
    maze();
  }
  else if ( _option == 2 )
  {
    // create crowd(pos, numAgent, gridSize, status)
    m_crowd.reset(new Crowd(ngl::Vec3(0,-5,0),6,m_gridSize,&m_status));
  }
  else if ( _option == 3 )
  {
    // create crowd(pos, numAgent, gridSize, status)
    m_crowd.reset(new Crowd(ngl::Vec3(0,-5,0),4,m_gridSize,&m_status));
  }
  else if ( _option == 4 )
  {
    // create crowd(pos, numAgent, gridSize, status)
    m_crowd.reset(new Crowd(ngl::Vec3(0,-5,0),5,m_gridSize,&m_status));

    // create bottleneck
    bottleneck();
  }

  // set camera and shader
  m_crowd->setCam(&m_cam);
  m_crowd->setShaderName("Phong");

  // set start and end position of agents
  m_crowd->setPoles(_option);

  // calculate routes for agents
  m_crowd->findRoute();
}

// sets a wall with an only gate
void NGLScene::bottleneck()
{
  m_crowd->grid()->setWall(ngl::Vec2(30,0), ngl::Vec2(30,25));
  m_crowd->grid()->setWall(ngl::Vec2(30,27), ngl::Vec2(30,49));
}

// sets a maze by using walls
void NGLScene::maze()
{
  ngl::Vec2 start;
  ngl::Vec2 end;

  // boundaries
  // right
  for ( int i=0; i<=9; i++ )
  {
    m_crowd->grid()->setWall(ngl::Vec2(15,i), ngl::Vec2(36,i));
  }

  m_values = {
    15,0,   15,45,  16,0,   16,45,  // top
    15,48,  35,48,  15,49,  35,49,  // left
    35,12,  35,49,  36,12,  36,49,  // bottom
    17,27,  17,26,  18,27,  18,26,  // 2 - vertical
    19,45,  19,40,  20,45,  20,40,  // 3
    19,33,  19,30,  20,33,  20,30,  // 3
    19,45,  19,40,  20,45,  20,40,  // 3
    19,23,  19,12,  20,23,  20,12,  // 3
    21,31,  21,28,  22,31,  22,28,  // 4
    21,45,  21,44,  22,45,  22,44,  // 4
    21,23,  21,22,  22,23,  22,22,  // 4
    23,45,  23,40,  24,45,  24,40,  // 5
    23,29,  23,26,  24,29,  24,26,  // 5
    23,13,  23,10,  24,13,  24,10,  // 5
    25,41,  25,32,  26,41,  26,32,  // 6
    25,27,  25,24,  26,27,  26,24,  // 6
    25,21,  25,16,  26,21,  26,16,  // 6
    27,49,  27,44,  28,49,  28,44,  // 7
    27,25,  27,20,  28,25,  28,20,  // 7
    27,41,  27,40,  28,41,  28,40,  // 7
    27,33,  27,32,  28,33,  28,32,  // 7
    29,37,  29,32,  30,37,  30,32,  // 8
    29,29,  29,28,  30,29,  30,28,  // 8
    29,21,  29,16,  30,21,  30,16,  // 8
    31,45,  31,36,  32,45,  32,36,  // 9
    31,29,  31,20,  32,29,  32,20,  // 9
    31,17,  31,16,  32,17,  32,16,  // 9
    31,13,  31,10,  32,13,  32,10,  // 9
    33,33,  33,32,  34,33,  34,32,  // 10
    33,25,  33,24,  34,25,  34,24,  // 10
    17,37,  26,37,  17,36,  26,36,  // 7 - horizontal
    21,17,  24,17,  21,16,  24,16,  // 17
    25,13,  28,13,  25,12,  28,12   // 19

  };

  for(unsigned int i = 0; i < m_values.size(); i+=4)
  {
    start=ngl::Vec2(m_values[i],m_values[i+1]);
    end=ngl::Vec2(m_values[i+2],m_values[i+3]);

    m_crowd->grid()->setWall(start, end);
  }
}


//---------------------------------------------------------------------------------

void NGLScene::loadMatricesToShader()
{
  ngl::ShaderLib *shader=ngl::ShaderLib::instance();

  ngl::Mat4 MV;
  ngl::Mat4 MVP;
  ngl::Mat3 normalMatrix;
  ngl::Mat4 M;
  M=m_mouseGlobalTX;
  MV=  M*m_cam.getViewMatrix();
  MVP= M*m_cam.getVPMatrix();
  normalMatrix=MV;
  normalMatrix.inverse();
  shader->setShaderParamFromMat4("MV",MV);
  shader->setShaderParamFromMat4("MVP",MVP);
  shader->setShaderParamFromMat3("normalMatrix",normalMatrix);
  shader->setShaderParamFromMat4("M",M);
}

//---------------------------------------------------------------------------------

void NGLScene::paintGL()
{
  glViewport(0,0,m_width,m_height);
  // clear the screen and depth buffer
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  
  // grab an instance of the shader manager
  ngl::ShaderLib *shader=ngl::ShaderLib::instance();
  (*shader)["Phong"]->use();

  // Rotation based on the mouse position for our global transform
  ngl::Mat4 rotX;
  ngl::Mat4 rotY;
  // create the rotation matrices
  rotX.rotateX(m_spinXFace);
  rotY.rotateY(m_spinYFace);
  // multiply the rotations
  m_mouseGlobalTX=rotY*rotX;
  // add the translations
  m_mouseGlobalTX.m_m[3][0] = m_modelPos.m_x;
  m_mouseGlobalTX.m_m[3][1] = m_modelPos.m_y;
  m_mouseGlobalTX.m_m[3][2] = m_modelPos.m_z;

  // draw the scene
  m_crowd->draw();

  m_crowd->drawGrid();

  m_crowd->drawObstacle();

}

//---------------------------------------------------------------------------------

void NGLScene::mouseMoveEvent (QMouseEvent * _event)
{
  // note the method buttons() is the button state when event was called
  // that is different from button() which is used to check which button was
  // pressed when the mousePress/Release event is generated
  if(m_rotate && _event->buttons() == Qt::LeftButton)
  {
    int diffx=_event->x()-m_origX;
    int diffy=_event->y()-m_origY;
    m_spinXFace += (float) 0.5f * diffy;
    m_spinYFace += (float) 0.5f * diffx;
    m_origX = _event->x();
    m_origY = _event->y();
    update();

  }
          // right mouse translate code
  else if(m_translate && _event->buttons() == Qt::RightButton)
  {
    int diffX = (int)(_event->x() - m_origXPos);
    int diffY = (int)(_event->y() - m_origYPos);
    m_origXPos=_event->x();
    m_origYPos=_event->y();
    m_modelPos.m_x += INCREMENT * diffX;
    m_modelPos.m_y -= INCREMENT * diffY;
    update();

   }
}

//---------------------------------------------------------------------------------

void NGLScene::mousePressEvent ( QMouseEvent * _event)
{
  // that method is called when the mouse button is pressed in this case we
  // store the value where the maouse was clicked (x,y) and set the Rotate flag to true
  if(_event->button() == Qt::LeftButton)
  {
    m_origX = _event->x();
    m_origY = _event->y();
    m_rotate =true;
  }
          // right mouse translate mode
  else if(_event->button() == Qt::RightButton)
  {
    m_origXPos = _event->x();
    m_origYPos = _event->y();
    m_translate=true;
  }

}

//---------------------------------------------------------------------------------

void NGLScene::mouseReleaseEvent ( QMouseEvent * _event )
{
  // that event is called when the mouse button is released
  // we then set Rotate to false
  if (_event->button() == Qt::LeftButton)
  {
    m_rotate=false;
  }
  // right mouse translate mode
  if (_event->button() == Qt::RightButton)
  {
    m_translate=false;
  }
}

//---------------------------------------------------------------------------------

void NGLScene::wheelEvent(QWheelEvent *_event)
{

	// check the diff of the wheel position (0 means no change)
	if(_event->delta() > 0)
	{
		m_modelPos.m_z+=ZOOM;
	}
	else if(_event->delta() <0 )
	{
		m_modelPos.m_z-=ZOOM;
	}
  update();
}

//---------------------------------------------------------------------------------

void NGLScene::keyPressEvent(QKeyEvent *_event)
{
  // that method is called every time the main window recives a key event.
  // we then switch on the key value and set the camera in the GLWindow
  switch (_event->key())
  {
  // escape key to quit
  case Qt::Key_Escape : QGuiApplication::exit(EXIT_SUCCESS); break;

  // change the status properties

  case Qt::Key_S : m_status.m_x=0; break;         // stop
  case Qt::Key_R : m_status.m_x=1; break;         // continue randomly
  case Qt::Key_C : m_status.m_x=-1; break;        // crowd movement

  case Qt::Key_Up : m_status.m_y=8; break;       // up
  case Qt::Key_Down : m_status.m_y=5; break;     // down
  case Qt::Key_Left : m_status.m_y=6; break;     // right
  case Qt::Key_Right : m_status.m_y=4; break;    // left

  case Qt::Key_Space : m_status.set(1,1,1); break;

  // show full screen or windowed
  case Qt::Key_F : showFullScreen(); break;
  case Qt::Key_N : showNormal(); break;
  default : break;
  }

  // update the GLWindow and re-draw
    update();
}

//---------------------------------------------------------------------------------

void NGLScene::timerEvent(QTimerEvent *_event )
{
  if(_event->timerId() ==   m_agentTimer)
  {
    m_crowd->update();
    update();
  }
}
