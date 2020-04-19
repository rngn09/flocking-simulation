#ifndef CROWD_H__
#define CROWD_H__

#include <vector>
#include <ngl/Vec3.h>
#include <list>

#include "Agent.h"
#include "Grid.h"


//---------------------------------------------------------------------------------

class Crowd
{
public :
  // _pos the position of the emitter
  // _numAgents the number of agents to create
  Crowd( ngl::Vec3 _pos, int _numAgents, int _gridSize, ngl::Vec3 *_status );
  ~Crowd() { delete m_grid; }

  // position update and draw the system
  void update();
  void draw();
  void drawGrid();
  void drawObstacle();

  // getters
  int numAgents() const {return m_numAgents;}
  Grid* grid() { return m_grid; }
  Agent& agent(int _i) { return m_agents[_i]; }

  // setters
  void setGrid(int _gridSize) { new Grid(_gridSize,_gridSize,1); }
  void setPoles(int _option);

  // route and collision
  void findRoute();
  void checkCollision(int _i);
  void stopAgent(int _i, int _step);
  void moveSideAgent(int _main, int _opponent, int _step);
  void checkNeighbours(int _row, int _col, int _ag, int _step);

  // camera and shader
  inline void setCam(ngl::Camera *_cam){m_cam=_cam;}
  inline ngl::Camera * getCam()const {return m_cam;}
  inline void setShaderName(const std::string &_n){m_shaderName=_n;}
  inline const std::string getShaderName()const {return m_shaderName;}


private :

  ngl::Vec3 m_pos;                        // position of the emitter
  int m_numAgents;                        // number of agents
  std::vector <Agent> m_agents;           // container for the agents
  ngl::Vec3 *m_status;                    // pointer to status
  std::string m_shaderName;               // name of the shader to use
  ngl::Camera *m_cam;                     // pointer to the camera used for drawing
  int currAgent = 0;                      // current agent
  Grid *m_grid;                           // pointer to grid
  std::vector <ngl::Vec2> m_tempRoute;    // temporary route
  bool m_occupied=false;                  // node occupied by an agent or obstacle
  ngl::Vec2 m_newStep=ngl::Vec2(-5,-5);   // to add new step to route if collision occurs
  std::vector<int> m_poles;               // start and end positions of agents


};


//---------------------------------------------------------------------------------



#endif

