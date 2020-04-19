#ifndef GRID_H__
#define GRID_H__

#include "Node.h"

class Grid
{
public :
  Grid() {}
  ~Grid() {}
  Grid(float _width, float _lenght, float _unit);

  // getters
  float getWidth() const {return m_width;}
  float getLenght() const {return m_lenght;}
  float getUnit() const {return m_unit;}
  int getColNum() const {return m_colNum;}
  int getRowNum() const {return m_rowNum;}
  int getNumNodes() const {return m_numNodes;}
  int getCount() const {return count;}
  int getNumObstacles() const {return m_numObstacles;}
  ngl::Vec2 getPath(int i) const {return m_path[i];}
  ngl::Vec2 getObstacles(int i) const {return m_obstacles[i];}
  ngl::Vec2 getEndNode() const {return m_endNode;}
  bool isTempObs(ngl::Vec2 _pos) {return m_nodes[index(_pos)].isObstacle();}
  bool isObstacle(ngl::Vec2 _pos) {return m_nodes[index(_pos)].isObstacle();}

  // setters
  void setStartNode();
  void setObstacles();
  void setCottage(ngl::Vec2 _pos);
  void setWall(ngl::Vec2 _first, ngl::Vec2 _last);


  // scores
  int calculateGScore(ngl::Vec2 _index);
  void calculateHScore(ngl::Vec2 _current);
  void calculateFScore(ngl::Vec2 _current);

  // loop
  void findNeighbours();
  void findMinFScore();
  void findPath(ngl::Vec2 _start, ngl::Vec2 _end);

  // grid
  void resetGrid();
  void drawGrid();

  // index
  int index(ngl::Vec2 _index);


private :

  float m_width;                        // width of grid
  float m_lenght;                       // lenght of grid
  float m_unit;                         // size of each node on the grid
  int m_colNum ;                        // number of columns on grid
  int m_rowNum ;                        // number of rows on grid
  int m_numNodes;                       // number of nodes on grid
  int count;                            // to measure the size of path
  std::vector <Node> m_nodes;           // vector of nodes
  std::vector <ngl::Vec2> m_path;       // vector of index of nodes that forms path
  ngl::Vec2 m_startNode;                // index of node of starting position
  ngl::Vec2 m_endNode;                  // index of node of target position
  ngl::Vec2 m_currentNode;              // index of node that is on the process atm
  std::vector <ngl::Vec2> m_obstacles;  // vector of index of nodes that occupied by obstacle
  std::vector <ngl::Vec2> m_tempPath;   // vector for a temporary path
  int m_numObstacles;                   // number of obstacles

};


#endif
