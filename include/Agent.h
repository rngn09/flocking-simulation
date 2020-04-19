#ifndef AGENT_H__
#define AGENT_H__

#include <vector>
#include <ngl/Camera.h>
#include <ngl/Vec3.h>

#include "Crowd.h"
#include "Agent.h"
#include "Node.h"
#include "Grid.h"


class Crowd;

//---------------------------------------------------------------------------------

class Agent
{
public :

  // _pos the start position of the agent
  Agent(ngl::Vec3 _pos,ngl::Vec3 *_status, Crowd *_crowd	);
  ~Agent() {}

  // position update and draw a certain agent
  void update();
  void draw();

  // getters
  ngl::Vec3 position() const {return m_pos;}
  ngl::Vec2 getStartPos() const {return m_startPos;}
  ngl::Vec2 getEndPos() const {return m_endPos;}
  ngl::Vec2 getRoute(int _step) const {return m_route[_step];}
  int getRouteSize() const {return m_routeSize;}
  int getSX() const {return m_sx;}
  int getSY() const {return m_sy;}
  int getEX() const {return m_ex;}
  int getEY() const {return m_ey;}

  // setters
  void setPosition(ngl::Vec3 _inVector) {m_pos =_inVector;}
  void setStartPos(ngl::Vec2 _start) {m_startPos=_start;}
  void setEndPos(ngl::Vec2 _end) {m_endPos=_end;}
  void setRoute(ngl::Vec2 _path);
  void setRouteSize(int _size) {m_routeSize=_size;}
  void setSX(int _sx) {m_sx=_sx;}
  void setSY(int _sy) {m_sy=_sy;}
  void setEX(int _ex) {m_ex=_ex;}
  void setEY(int _ey) {m_ey=_ey;}

  // method to clean the former route
  void cleanRoute();


private :

  ngl::Vec3 m_pos;                 // current agent position
  ngl::Vec3 *m_status;             // pointer to status vector
  const Crowd *m_crowd;            // pointer to crowd
  float step = 0.05;               // step length
  int m_routeSize;                 // size of route
  int m_sx,m_sy,m_ex,m_ey;         // start and end values for steps
  int queue=0;                     // number of current node in the route
  int time=0;                      // waiting period
  std::vector<ngl::Vec2> m_route;  // vector of nodes that forms route
  ngl::Vec2 m_startPos;            // start position of agent
  ngl::Vec2 m_endPos;              // end position of agent


};


//---------------------------------------------------------------------------------



#endif

