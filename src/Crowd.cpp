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


//_pos->position of emitter
Crowd::Crowd(ngl::Vec3 _pos, int _numAgents, int _gridSize,
             ngl::Vec3 *_status )
{
  m_status=_status;
  m_pos=_pos;

  // create agents
  for (int i=0; i< _numAgents; ++i)
  {
    m_agents.push_back(Agent(_pos,m_status,this));
  }

  setPoles(2);
   m_numAgents=_numAgents;

  // create grid
  m_grid = new Grid(_gridSize,_gridSize,1);
}


//---------------------------------------------------------------------------------


void Crowd::setPoles(int _option)
{
  ngl::Vec2 start, end;
  int a,b,c,d;

  // option 1: maze
  if ( _option == 1 )
  {
    m_poles={ 9,47,   49,49,
              38,14,  0,0    };

    for( int i = 0; i < 2; i++)
    {
      a=(i*4)+0;
      b=(i*4)+1;
      c=(i*4)+2;
      d=(i*4)+3;

      start=ngl::Vec2(m_poles[a],m_poles[b]);
      end=ngl::Vec2(m_poles[c],m_poles[d]);

      m_agents[i].setStartPos(start);
      m_agents[i].setEndPos(end);
    }
//    // clean m_poles
//    while ( !m_poles.empty() )
//    {
//      m_poles.pop_back();
//    }

  }
  // option 2: towards collision
  else if ( _option == 2 )
  {
    m_poles={ 20,30,  48,30,
              30,30,   0,30,
              10,30,  48,30,
              40,30,   0,30,
               0,30,  48,30,
              48,30,   0,30   };

    for( int i = 0; i < 6; i++)
    {
      a=(i*4)+0;
      b=(i*4)+1;
      c=(i*4)+2;
      d=(i*4)+3;

      start=ngl::Vec2(m_poles[a],m_poles[b]);
      end=ngl::Vec2(m_poles[c],m_poles[d]);

      m_agents[i].setStartPos(start);
      m_agents[i].setEndPos(end);
    }
  }
  // option 3: glancing collision
  else if ( _option == 3 )
  {
    m_poles={ 10,40,  48,40,
              20,30,  20,48,
              30,20,  30,48,
              40,10,  40,48  };

    for( int i = 0; i < 4; i++)
    {
      a=(i*4)+0;
      b=(i*4)+1;
      c=(i*4)+2;
      d=(i*4)+3;

      start=ngl::Vec2(m_poles[a],m_poles[b]);
      end=ngl::Vec2(m_poles[c],m_poles[d]);

      m_agents[i].setStartPos(start);
      m_agents[i].setEndPos(end);
    }
  }
  // option 4: bottleneck
  else if ( _option == 4 )
  {
    m_poles={ 20,30,  48,30,
              20,23,  48,23,
              20,21,  48,21,
              20,32,  48,32,
              20,28,  48,28   };

    for( int i = 0; i < 5; i++)
    {
      a=(i*4)+0;
      b=(i*4)+1;
      c=(i*4)+2;
      d=(i*4)+3;

      start=ngl::Vec2(m_poles[a],m_poles[b]);
      end=ngl::Vec2(m_poles[c],m_poles[d]);

      m_agents[i].setStartPos(start);
      m_agents[i].setEndPos(end);
    }
  }
}


//---------------------------------------------------------------------------------


void Crowd::findRoute()
{
  // loop to check each agent in the crowd
  for (int i=0; i<m_numAgents; ++i)
  {
    // find path for ith agent
    m_grid->findPath(m_agents[i].getStartPos(), m_agents[i].getEndPos());

    // get route size from findPath function
    m_agents[i].setRouteSize(m_grid->getCount());

    // copy the route to store within the agent
    for(int j = 0; j < m_grid->getCount(); j++)
    {
      m_agents[i].setRoute(m_grid->getPath(j));

    m_agents[i].setEX(m_grid->getPath(0).m_x);
    m_agents[i].setEY(m_grid->getPath(0).m_y);

    }
  }
  // check if any collision occurs between agents
  for (int i=1; i< m_numAgents; ++i)
  {
    checkCollision(i);
  }
}


//---------------------------------------------------------------------------------


void Crowd::checkCollision(int _i)
{
  // for each step of agent check if there is collision with
  // one of other agents
  for ( int step=0; step<m_agents[_i].getRouteSize() ; step++ )
  {
    for ( int j=0; j<_i; j++ )
    {
      // if there is two agent in a node in any time step
      if ( m_agents[_i].getRoute(step) == m_agents[j].getRoute(step) )
      {
        // if agents are on the path of each other move side
        if ( m_agents[_i].getRoute(step-1) == m_agents[j].getRoute(step+1) )
        {
          moveSideAgent(_i, j, step);
        }
        // if not stop the first agent to wait the other one pass
        else
        {
          stopAgent(_i, step);
        }
      }
      // if agents are moving forward to each others current position
      // first agent will move side to give way, second one will move forward
      else if ( m_agents[_i].getRoute(step) == m_agents[j].getRoute(step+1) &&
                m_agents[_i].getRoute(step+1) == m_agents[j].getRoute(step)  )
      {
        moveSideAgent(_i, j, step);
      }
    }
  }
}


//---------------------------------------------------------------------------------


void Crowd::stopAgent(int _i, int _step)
{
  // copy the route of current agent to a temporary vector
  for ( int k=0; k<m_agents[_i].getRouteSize(); k++ )
  {
    m_tempRoute.push_back(m_agents[_i].getRoute(k));

    // when it comes to current time step add the previous node again
    // to make the current agent stop until the other one pass
    if ( k == _step-1 )
    {
      m_tempRoute.push_back(m_agents[_i].getRoute(k));
    }
  }
  // clean original route
  m_agents[_i].cleanRoute();

  // copy new route from temporary vector to original
  for ( unsigned int k=0; k<m_tempRoute.size(); k++ )
  {
    m_agents[_i].setRoute(m_tempRoute[k]);
  }
  // clean temporary vector
  while ( !m_tempRoute.empty() )
  {
    m_tempRoute.pop_back();
  }
}


//---------------------------------------------------------------------------------


// this function will move the agent j to side
// agent i is just here for compare

void Crowd::moveSideAgent(int _main, int _opponent, int _step)
{
  // copy the route of current agent to the temporary vector

  for ( int k=0; k<m_agents[_main].getRouteSize(); k++ )
  {
    m_tempRoute.push_back(m_agents[_main].getRoute(k));

    // when it comes to current time step add a new step to move side
    if ( k == _step-1 )
    {
      checkNeighbours(m_agents[_main].getRoute(k).m_x,m_agents[_main].getRoute(k).m_y,_main,_step);

      // add this step to path
      m_tempRoute.push_back(m_newStep);
      m_tempRoute.push_back(m_newStep);
      m_tempRoute.push_back(m_newStep);

      // add the step to turn back to original path
      m_newStep= ngl::Vec2(m_agents[_main].getRoute(k).m_x, m_agents[_main].getRoute(k).m_y);

      m_tempRoute.push_back(m_newStep);
    }
  }

  // clean original route
  m_agents[_main].cleanRoute();

  // copy new route from temporary vector to original
  for ( unsigned int k=0; k<m_tempRoute.size(); k++ )
  {
    m_agents[_main].setRoute(m_tempRoute[k]);
  }
  // clean temporary vector
  while ( !m_tempRoute.empty() )
  {
    m_tempRoute.pop_back();
  }
}


//---------------------------------------------------------------------------------

// _ag current agent number
// _row and _col of current position
void Crowd::checkNeighbours(int _row, int _col, int _ag, int _step)
{
  int count=0;
  m_newStep=ngl::Vec2(_row,_col);

  // roam neighbour nodes
  for ( int j=_col-1; j<=_col+1; j++ )
  {
    for ( int i=_row-1; i<=_row+1; i++ )
    {
      // if it is the same node skip
      if ( i == _row && j == _col )
      {
        continue;
      }
      // if node is on the current way of agent skip
      if ( (ngl::Vec2(i,j) == m_agents[_ag].getRoute(_step))   ||
           (ngl::Vec2(i,j) == m_agents[_ag].getRoute(_step+1)) ||
           (ngl::Vec2(i,j) == m_agents[_ag].getRoute(_step-1)) ||
           (ngl::Vec2(i,j) == m_agents[_ag].getRoute(_step-2))   )
      {
        continue;
      }
      // check if the node is already occupied by any other agent
      for ( int j=0; j<m_numAgents; j++ )
      {
        if ( ngl::Vec2(i,j) == m_agents[j].getRoute(_step) )
        {
          m_occupied=true;
        }
      }
      // check if the node is already occupied by an obstacle
      ngl::Vec2 index=ngl::Vec2(i,j);

      if ( m_grid->isObstacle(index) == true )
      {
        m_occupied=true;
      }
      // if the node is avaliable and closer to the next step of agent
      // change the newStep to this node
      if ( m_occupied == false )
      {
        if ( abs(_row-i) <= abs(_row-m_newStep.m_x) )
        {
          m_newStep=ngl::Vec2(i,j);
        }
      }
      else if ( m_occupied == true )
      {
        count++;
      }
      // if all of the neighbours are occupied wait at current position
      if ( count == 8 )
      {
        m_newStep=ngl::Vec2(_row,_col);
      }
    }
  }
}


//---------------------------------------------------------------------------------


// update each of the agents position in the system
void Crowd::update()
{
  for(int i=0; i<m_numAgents; ++i)
  {
    currAgent = i;

      m_agents[i].update();
  }
}


//---------------------------------------------------------------------------------


// draw all the agents in the system
void Crowd::draw()
{
  for(int i=0; i<m_numAgents; ++i)
  {
    m_agents[i].draw();
  }
}


//---------------------------------------------------------------------------------


void Crowd::drawGrid()
{
  float x,y;
  x=(m_grid->getWidth()-m_grid->getUnit())/2;
  y=(m_grid->getLenght()-m_grid->getUnit())/2;

  // get the VBO instance and draw the built in sphere
  ngl::VAOPrimitives *prim=ngl::VAOPrimitives::instance();
  ngl::Transformation transform;
  ngl::ShaderLib *shader=ngl::ShaderLib::instance();
  shader->use(m_shaderName);
  transform.setPosition(ngl::Vec3(x,0,y));

  ngl::Mat4 MV;
  ngl::Mat4 MVP;
  ngl::Mat3 normalMatrix;
  ngl::Mat4 M;
  M=transform.getMatrix();
  MV=transform.getMatrix()*m_cam->getViewMatrix();
  MVP=MV*m_cam->getProjectionMatrix() ;
  normalMatrix=MV;
  normalMatrix.inverse();
  shader->setShaderParamFromMat4("MV",MV);
  shader->setShaderParamFromMat4("MVP",MVP);
  shader->setShaderParamFromMat3("normalMatrix",normalMatrix);
  shader->setShaderParamFromMat4("M",M);

  prim->draw("Grid");
}


//---------------------------------------------------------------------------------


void Crowd::drawObstacle()
{
  float x,y;

  for (int i=0; i<m_grid->getNumObstacles(); i++)
  {
  x=m_grid->getObstacles(i).m_x;
  y=m_grid->getObstacles(i).m_y;

  // get the VBO instance and draw the built in sphere
  ngl::VAOPrimitives *prim=ngl::VAOPrimitives::instance();
  ngl::Transformation transform;
  ngl::ShaderLib *shader=ngl::ShaderLib::instance();
  shader->use(m_shaderName);
  transform.setPosition(ngl::Vec3(x,-0.2,y));

  ngl::Mat4 MV;
  ngl::Mat4 MVP;
  ngl::Mat3 normalMatrix;
  ngl::Mat4 M;
  M=transform.getMatrix();
  MV=transform.getMatrix()*m_cam->getViewMatrix();
  MVP=MV*m_cam->getProjectionMatrix() ;
  normalMatrix=MV;
  normalMatrix.inverse();
  shader->setShaderParamFromMat4("MV",MV);
  shader->setShaderParamFromMat4("MVP",MVP);
  shader->setShaderParamFromMat3("normalMatrix",normalMatrix);
  shader->setShaderParamFromMat4("M",M);

  prim->draw("cube");
  }
}


//---------------------------------------------------------------------------------
