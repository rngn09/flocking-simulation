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


Node::Node(ngl::Vec3 _position, Grid *_grid)
{
  m_position=_position;
  m_grid=_grid;
}


//---------------------------------------------------------------------------------




