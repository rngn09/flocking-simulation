#include <iostream>
#include <cstdlib>
#include <math.h>
#include <memory>

#include <ngl/Camera.h>
#include <ngl/Random.h>
#include <ngl/Transformation.h>
#include <ngl/ShaderLib.h>
#include <ngl/VAOPrimitives.h>

#include "Crowd.h"
#include "Agent.h"
#include "Node.h"
#include "Grid.h"


//---------------------------------------------------------------------------------


//_pos->first position of agent
Agent::Agent(ngl::Vec3 _pos, ngl::Vec3 *_status,  Crowd *_crowd  )
{
  m_pos=_pos;
  m_status=_status;
  m_crowd=_crowd;

  // set start and end position to calculate path

//  ngl::Random *rand=ngl::Random::instance();
//  m_startPos.m_x=int(rand->randomPositiveNumber(1)*100)%49;
//  m_startPos.m_y=int(rand->randomPositiveNumber(1)*100)%49;
//  m_endPos.m_x=int(rand->randomPositiveNumber(1)*100)%49;
//  m_endPos.m_y=int(rand->randomPositiveNumber(1)*100)%49;

  m_startPos.m_x=40;
  m_startPos.m_y=45;
  m_endPos.m_x=49;
  m_endPos.m_y=49;

  // set start position as current position of agent
  m_pos.m_x=m_startPos.m_x;
  m_pos.m_y=0;
  m_pos.m_z=m_startPos.m_y;

  // set varibles as consecutive members of path range
  m_sx=m_pos.m_x;
  m_sy=m_pos.m_z;
}


//---------------------------------------------------------------------------------


void Agent::setRoute(ngl::Vec2 _path)
{
  m_route.push_back(_path);
}


//---------------------------------------------------------------------------------


void Agent::cleanRoute()
{
  while ( !m_route.empty() )
  {
      m_route.pop_back();
  }

}

//---------------------------------------------------------------------------------


// update the agent position
void Agent::update()
{
  // step measure - also for changing the speed
  step = 0.05;

  // move until reach the next node in the path
  if ((m_sx > m_ex && m_pos.m_x >= m_ex) ||
      (m_sy > m_ey && m_pos.m_z >= m_ey) ||
      (m_sx < m_ex && m_pos.m_x <= m_ex) ||
      (m_sy < m_ey && m_pos.m_z <= m_ey) )
  {

    m_pos.m_x=m_pos.m_x+((m_ex-m_sx)*step);
    m_pos.m_y=0;
    m_pos.m_z=m_pos.m_z+((m_ey-m_sy)*step);

  }
  else
  {
    // when reach a node check if the next node is the same
    // if yes wait for a while
    if ( ( m_ex == m_route[queue+1].m_x &&
           m_ey == m_route[queue+1].m_y ) &&
         ( time < 30 ) )
    {
      time++;
    }
    // if next node is different than
    // change the start and end nodes to start moving next node
    else if ( queue < m_routeSize-1)
    {
      queue++;

      m_sx=m_ex;
      m_sy=m_ey;

      m_ex=m_route[queue].m_x;
      m_ey=m_route[queue].m_y;

      time=0;
    }
  }
}

//---------------------------------------------------------------------------------


// draw the agent
void Agent::draw()
{
  // get the VBO instance and draw the built in sphere
  ngl::VAOPrimitives *prim=ngl::VAOPrimitives::instance();
  ngl::Transformation transform;
  ngl::ShaderLib *shader=ngl::ShaderLib::instance();
  shader->use(m_crowd->getShaderName());
  transform.setPosition(m_pos);


  ngl::Mat4 MV;
  ngl::Mat4 MVP;
  ngl::Mat3 normalMatrix;
  ngl::Mat4 M;
  M=transform.getMatrix();
  MV=transform.getMatrix()*m_crowd->getCam()->getViewMatrix();
  MVP=MV*m_crowd->getCam()->getProjectionMatrix() ;
  normalMatrix=MV;
  normalMatrix.inverse();
  shader->setShaderParamFromMat4("MV",MV);
  shader->setShaderParamFromMat4("MVP",MVP);
  shader->setShaderParamFromMat3("normalMatrix",normalMatrix);
  shader->setShaderParamFromMat4("M",M);
  prim->draw("sphere");

}


//---------------------------------------------------------------------------------
