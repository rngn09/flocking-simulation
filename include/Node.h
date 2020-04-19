#ifndef NODE_H__
#define NODE_H__

#include <vector>
#include <ngl/Vec3.h>

class Grid;


//---------------------------------------------------------------------------------


class Node
{
public :

  Node() {}
  Node(ngl::Vec3 _position, Grid *_grid);

  // getters
  const ngl::Vec3 &getPosition() const{return m_position;}
  const ngl::Vec2 getParent() const{return m_parent;}
  const int &getRow() const{return m_row;}
  const int &getCol() const{return m_col;}
  const int &getFScore() const{return m_fScore;}
  const int &getGScore() const{return m_gScore;}
  const int &getHScore() const{return m_hScore;}
  bool isOpen() const{return m_open;}
  bool isClose() const{return m_close;}
  bool isObstacle() const{return m_obstacle;}

  // setters
  void setPosition(ngl::Vec3 _position) {m_position=_position;}
  void setParent(ngl::Vec2 _parent) {m_parent=_parent;}
  void setRow(int _row) {m_row=_row;}
  void setCol(int _col) {m_col=_col;}
  void setGScore(int _gScore) {m_gScore=_gScore;}
  void setHScore(int _hScore) {m_hScore=_hScore;}
  void setFScore(int _fScore) {m_fScore=_fScore;}
  void setOpen(bool _open) {m_open=_open;}
  void setClose(bool _close) {m_close=_close;}
  void setObstacle(bool _obstacle) {m_obstacle=_obstacle;}


private :

  ngl::Vec3 m_position;   // position of node
  ngl::Vec2 m_parent;     // index of parent node
  int m_gScore=0;         // g score of node - taken
  int m_hScore;           // h score of node - remain
  int m_fScore;           // f score of node - taken + remain
  int m_row;              // row index of node
  int m_col;              // column index of node
  bool m_open;            // open status of node
  bool m_close;           // closed status of node
  bool m_obstacle=false;  // occupation status of node
  Grid *m_grid;           // pointer to grid

};


//---------------------------------------------------------------------------------


#endif
