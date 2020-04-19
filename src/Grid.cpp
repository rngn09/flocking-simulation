#include <iostream>
#include <cstdlib>
#include <math.h>
#include <memory>

#include <ngl/Camera.h>
#include <ngl/Random.h>
#include <ngl/Transformation.h>
#include <ngl/ShaderLib.h>
#include <ngl/VAOPrimitives.h>
#include "limits.h"

#include "Grid.h"
#include "Crowd.h"


//---------------------------------------------------------------------------------


Grid::Grid(float _width, float _lenght, float _unit)
{
  // assign the grid and node values

  m_width=_width;
  m_lenght=_lenght;
  m_unit=_unit;

  // calculate the number of rows and column of the grid

  m_colNum = (m_width/m_unit);
  m_rowNum = (m_lenght/m_unit);

  m_numNodes = m_colNum*m_rowNum;

  // create nodes
  // index of node(i,j)=node(i*m_colNum+j)
  for (int i=0; i< m_rowNum; ++i)
  {
    for (int j=0; j< m_colNum; ++j)
    {
      Node node;
      m_nodes.push_back(node);
    }
  }
  // set attiributes of nodes
  for (int i=0; i< m_rowNum; ++i)
  {
    for (int j=0; j< m_colNum; ++j)
    {
      int ix=index(ngl::Vec2(i,j));

      m_nodes[ix].setPosition(ngl::Vec3(m_unit*i, 0, m_unit*j));
      m_nodes[ix].setRow(i);
      m_nodes[ix].setCol(j);

      // reset grid to default attiributes
      resetGrid();
    }
  }
}


//---------------------------------------------------------------------------------

//*************************** !!WARNING!! *************************
// there is something wrong here.
// it supposed to write as obstacle if m_obstacle is true
// but it does not work as it should. WHY?

void Grid::setObstacles()
{
  // find and add the nodes that occupied by an obstacle to the vector
  for (int i=0; i<m_numNodes; i++)
  {
    if (m_nodes[i].isObstacle() != true )
    {
//      m_obstacles.push_back(ngl::Vec2(m_nodes[i].getRow(),m_nodes[i].getCol()));
    }
    else
    {
      m_obstacles.push_back(ngl::Vec2(m_nodes[i].getRow(),m_nodes[i].getCol()));
    }
  }

  m_numObstacles=m_obstacles.size();

}


//---------------------------------------------------------------------------------


void Grid::setCottage(ngl::Vec2 _pos)
{
  // sets obstacles to node itself and all neighbours of
  // a node that corresponds given position
  for (int i=_pos.m_x-1; i<=_pos.m_x+1; i++)
  {
    for (int j=_pos.m_y-1; j<=_pos.m_y+1; j++)
    {
      m_nodes[index(ngl::Vec2(i,j))].setObstacle(true);
    }
  }
}


//---------------------------------------------------------------------------------


void Grid::setWall(ngl::Vec2 _first, ngl::Vec2 _last)
{
  int minor=0;
  int major=0;

  // throw an error if the first and last node of wall are
  // not in same row or column
  if ( _first.m_x != _last.m_x && _first.m_y != _last.m_y )
  {
    std::cout<<"ERROR: First and last point of wall have to be in same row or column!"<<std::endl;
  }
  // if it is a wall located throghout a column
  else if ( _first.m_x != _last.m_x )
  {
    if (_first.m_x > _last.m_x)
    {
      minor= _last.m_x;
      major= _first.m_x;
    }
    else
    {
      minor= _first.m_x;
      major= _last.m_x;
    }

    for( int i=minor; i<=major; i++ )
    {
      m_nodes[index(ngl::Vec2(i,_last.m_y))].setObstacle(true);
    }
  }
  // if it is a wall located throghout a row
  else if ( _first.m_y != _last.m_y )
  {
    if (_first.m_y > _last.m_y)
    {
      minor= _last.m_y;
      major= _first.m_y;
    }
    else
    {
      minor= _first.m_y;
      major= _last.m_y;
    }

    for( int i=minor; i<=major; i++ )
    {
      m_nodes[index(ngl::Vec2(_last.m_x,i))].setObstacle(true);
    }
  }
}


//---------------------------------------------------------------------------------


void Grid::findPath(ngl::Vec2 _start, ngl::Vec2 _end)
{
  m_startNode=_start;
  m_endNode=_end;

  setObstacles();
  setStartNode();

  // loop until reach the target
  while ( m_currentNode != m_endNode )
    {
      findNeighbours();
      findMinFScore();
    }
  // create a temporary variable for changes
  ngl::Vec2 temp_curr=m_currentNode;
  count=1;

  // count the number of nodes on the path
  while ( m_nodes[index(temp_curr)].getParent() != ngl::Vec2(-1,-1) )
  {
    temp_curr=m_nodes[index(temp_curr)].getParent();

    count++;
  }
  // reset the temporary variable
  temp_curr=m_currentNode;

  // store the index data of path in a vector
  for (int i=count-1; i>=0; i--)
  {
    m_path.push_back(ngl::Vec2());
  }

  for (int i=count-1; i>=0; i--)
  {
    m_path[i]=temp_curr;

    temp_curr=m_nodes[index(temp_curr)].getParent();
  }
  // create a temporary path
  // while it is moving diagonally, if there is an obstacle
  // add a new step that turns corner
  for ( int i=0; i<count; i++ )
  {
    m_tempPath.push_back(m_path[i]);

    // if x2>x1 and y2>y1
    if ( m_path[i].m_x < m_path[i+1].m_x && m_path[i].m_y < m_path[i+1].m_y )
    {
      if ( m_nodes[index(ngl::Vec2(m_path[i].m_x+1,m_path[i].m_y))].isObstacle() == true  )
      {
        m_tempPath.push_back(ngl::Vec2(m_path[i].m_x, m_path[i].m_y+1));
      }
      else if ( m_nodes[index(ngl::Vec2(m_path[i].m_x,m_path[i].m_y+1))].isObstacle() == true )
      {
        m_tempPath.push_back(ngl::Vec2(m_path[i].m_x+1, m_path[i].m_y));
      }
    }
    // if x2<x1 and y2<y1
    else if ( m_path[i].m_x > m_path[i+1].m_x && m_path[i].m_y > m_path[i+1].m_y )
    {
      if ( m_nodes[index(ngl::Vec2(m_path[i].m_x-1,m_path[i].m_y))].isObstacle() == true  )
      {
        m_tempPath.push_back(ngl::Vec2(m_path[i].m_x, m_path[i].m_y-1));
      }
      else if ( m_nodes[index(ngl::Vec2(m_path[i].m_x,m_path[i].m_y-1))].isObstacle() == true )
      {
        m_tempPath.push_back(ngl::Vec2(m_path[i].m_x-1, m_path[i].m_y));
      }
    }
    // if x2>x1 and y2<y1
    else if ( m_path[i].m_x < m_path[i+1].m_x && m_path[i].m_y > m_path[i+1].m_y )
    {
      if ( m_nodes[index(ngl::Vec2(m_path[i].m_x+1,m_path[i].m_y))].isObstacle() == true  )
      {
        m_tempPath.push_back(ngl::Vec2(m_path[i].m_x, m_path[i].m_y-1));
      }
      else if ( m_nodes[index(ngl::Vec2(m_path[i].m_x,m_path[i].m_y-1))].isObstacle() == true )
      {
        m_tempPath.push_back(ngl::Vec2(m_path[i].m_x+1, m_path[i].m_y));
      }
    }
    // if x2<x1 and y2>y1
    else if ( m_path[i].m_x > m_path[i+1].m_x && m_path[i].m_y < m_path[i+1].m_y )
    {
      if ( m_nodes[index(ngl::Vec2(m_path[i].m_x-1,m_path[i].m_y))].isObstacle() == true  )
      {
        m_tempPath.push_back(ngl::Vec2(m_path[i].m_x, m_path[i].m_y+1));
      }
      else if ( m_nodes[index(ngl::Vec2(m_path[i].m_x,m_path[i].m_y+1))].isObstacle() == true )
      {
        m_tempPath.push_back(ngl::Vec2(m_path[i].m_x-1, m_path[i].m_y));
      }
    }
  }
  // clean original path
  while ( !m_path.empty() )
  {
    m_path.pop_back();
  }
  // copy the new path to original vector
  for ( unsigned int i=0; i<m_tempPath.size(); i++ )
  {
    m_path.push_back(m_tempPath[i]);
  }
  // clean the temporary path
  while ( !m_tempPath.empty() )
  {
    m_tempPath.pop_back();
  }
  // reset grid to default attributes to prepare next agents route
  resetGrid();
}


//---------------------------------------------------------------------------------


void Grid::resetGrid()
{
  // default attributes of grid
  for ( int i=0; i<=m_numNodes; i++ )
  {
    m_nodes[i].setFScore(0);
    m_nodes[i].setGScore(0);
    m_nodes[i].setOpen(false);
    m_nodes[i].setClose(false);
    m_nodes[i].setParent(ngl::Vec2(-1,-1));
  }
}


//---------------------------------------------------------------------------------


// find the node with lowest f score in open list
void Grid::findMinFScore()
{
  // create temporary variables for changes
  int minix;
  int minFScore=INT_MAX;
  int minHScore=0;

  // find the nodes that signed as open in nodes list
  for ( int i=0; i<= m_numNodes; i++ )
  {
    if( m_nodes[i].isOpen() == true )
    {
      // check if its f score lesser then minFScore
      // if the f scores are equal check for the h scores

      if ( m_nodes[i].getFScore() < minFScore )
      {
        minFScore=m_nodes[i].getFScore();
        minHScore=m_nodes[i].getHScore();
        minix=i;
      }
      else if ( m_nodes[i].getFScore() == minFScore )
      {
        if ( m_nodes[i].getHScore() < minHScore )
        {
          minFScore=m_nodes[i].getFScore();
          minHScore=m_nodes[i].getHScore();
          minix=i;
        }
      }
    }
  }
  // move current_node from open list to closed list
  m_nodes[index(m_currentNode)].setOpen(false);
  m_nodes[index(m_currentNode)].setClose(true);

  // set the node with min f score as current_node
  m_currentNode=ngl::Vec2(m_nodes[minix].getRow(),m_nodes[minix].getCol());
}


//---------------------------------------------------------------------------------


// find the neighbours of the current_node and add to open list
void Grid::findNeighbours()
{
  for ( int i =  m_nodes[index(m_currentNode)].getRow()-1;
            i <= m_nodes[index(m_currentNode)].getRow()+1; i++ )
  {
    for ( int j =  m_nodes[index(m_currentNode)].getCol()-1;
              j <= m_nodes[index(m_currentNode)].getCol()+1; j++ )
    {
      // skip if the neighbour node out of range
      if ( i <0 || i>=m_rowNum || j<0 || j>=m_colNum )
      {
        continue;
      }
      // skip if the neighbour node equals to current node
      if ( i == m_nodes[index(m_currentNode)].getRow() &&
           j == m_nodes[index(m_currentNode)].getCol() )
      {
        continue;
      }
      // check if the neighbour node already evaluated or obstacle
      if ( m_nodes[index(ngl::Vec2(i,j))].isClose() == false &&
           m_nodes[index(ngl::Vec2(i,j))].isObstacle() == false )
      {
        // calculate current G Score of neighbour
        int g_score=calculateGScore(ngl::Vec2(i,j));

        // if the neighbour node already in open list and
        // current G Score lesser than existing, update
        // the G Score and the parent
        if ( m_nodes[index(ngl::Vec2(i,j))].isOpen() == true &&
             g_score < m_nodes[index(ngl::Vec2(i,j))].getGScore() )
        {
          m_nodes[index(ngl::Vec2(i,j))].setGScore(g_score);
          m_nodes[index(ngl::Vec2(i,j))].setParent(ngl::Vec2(m_nodes[index(m_currentNode)].getRow(),
                                                             m_nodes[index(m_currentNode)].getCol()));
          calculateHScore(ngl::Vec2(i,j));
          calculateFScore(ngl::Vec2(i,j));
        }
        // else set the parent and G Score
        // add the neighbour to open list
        else if ( m_nodes[index(ngl::Vec2(i,j))].isOpen() == false )
        {
          m_nodes[index(ngl::Vec2(i,j))].setGScore(g_score);
          m_nodes[index(ngl::Vec2(i,j))].setParent(ngl::Vec2(m_nodes[index(m_currentNode)].getRow(),
                                                             m_nodes[index(m_currentNode)].getCol()));
          calculateHScore(ngl::Vec2(i,j));
          calculateFScore(ngl::Vec2(i,j));

          m_nodes[index(ngl::Vec2(i,j))].setOpen(true);

          // move current node to the closed list
          m_nodes[index(m_currentNode)].setOpen(false);
          m_nodes[index(m_currentNode)].setClose(true);
        }
      }
    }
  }
}


//---------------------------------------------------------------------------------


// set start node
void Grid::setStartNode()
{
  int ix=index(m_startNode);

  // start node has no parent so
  // give it an impossible node as parent
  m_nodes[ix].setParent(ngl::Vec2(-1,-1));  // null

  // set G score as zero
  // calculate F and H score
  m_nodes[ix].setGScore(0);
  calculateHScore(m_startNode);
  calculateFScore(m_startNode);

  // add start node to open list
  m_nodes[ix].setOpen(true);

  // set start node as current node
  m_currentNode = m_startNode;
}


//---------------------------------------------------------------------------------


int Grid::calculateGScore(ngl::Vec2 _index)
{
  // distance is 14 for diagonal neighbours and 10 for others
  int distance;

  if ( _index.m_x == m_nodes[index(m_currentNode)].getRow() ||
       _index.m_y == m_nodes[index(m_currentNode)].getCol() )
  {
    distance=10;
  }
  else
  {
    distance=14;
  }
  // g score of the node is the g score of parent + distance
  int g_score=m_nodes[index(m_currentNode)].getGScore()+distance;

  return g_score;
}


//---------------------------------------------------------------------------------


void Grid::calculateHScore(ngl::Vec2 _current)
{
  // moving diagonal takes less effort than moving vertical and horizontal
  // so calculate the diagonal distance first
  int diagonal=0;
  int vertical=0;

  ngl::Vec2 current=_current;

  // move diagonally until one of the index components equal to target's
  // count the number of diagonal steps
  while (current.m_x != m_endNode.m_x && current.m_y != m_endNode.m_y)
  {
    if ( current.m_x < m_endNode.m_x && current.m_y < m_endNode.m_y )
    {
      current.m_x++;
      current.m_y++;
    }
    else if ( current.m_x > m_endNode.m_x && current.m_y < m_endNode.m_y )
    {
      current.m_x--;
      current.m_y++;
    }
    else if ( current.m_x < m_endNode.m_x && current.m_y > m_endNode.m_y )
    {
      current.m_x++;
      current.m_y--;
    }
    else if ( current.m_x > m_endNode.m_x && current.m_y > m_endNode.m_y )
    {
      current.m_x--;
      current.m_y--;
    }
    diagonal++;
  }
  // move vertically or horizontally both components equal to target's
  // count the number of vertical/horizontal steps
  while ( current.m_x != m_endNode.m_x || current.m_y != m_endNode.m_y )
  {
    if ( current.m_x < m_endNode.m_x )
    {
      current.m_x++;
    }
    else if ( current.m_x > m_endNode.m_x )
    {
      current.m_x--;
    }
    else if ( current.m_y < m_endNode.m_y )
    {
      current.m_y++;
    }
    else if ( current.m_y > m_endNode.m_y )
    {
      current.m_y--;
    }
    vertical++;
  }
  // multiply diagonal steps with 14 and vertical/horizontal steps with 10
  // add results together to find hScore
  int hScore = vertical*10+diagonal*14;

  // set H score of the node
  m_nodes[index(_current)].setHScore(hScore);
}


//---------------------------------------------------------------------------------


void Grid::calculateFScore(ngl::Vec2 _current)
{
  // f score gets calculated by adding h score and g score together
  int fScore=m_nodes[index(_current)].getGScore() +
             m_nodes[index(_current)].getHScore();

  // set f score of the node
  m_nodes[index(_current)].setFScore(fScore);
}


//---------------------------------------------------------------------------------


int Grid::index(ngl::Vec2 _index)
{
  // (i,j)=i*colNum+j
  int index=_index.m_x*m_colNum+_index.m_y;

  return index;
}


//---------------------------------------------------------------------------------

