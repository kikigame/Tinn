/* License and copyright go here*/

#include "transport.hpp"

transport::transport(terrainType activate, 
		     terrainType allow) :
  terrainToActivate_(*tFactory.get(activate)),
  terrainToAllow_(*tFactory.get(allow)) {}
transport::~transport() {}
const terrain &transport::terrainFor(const terrain &base) const {
  return base.type() == terrainToActivate_.type() ? terrainToAllow_ : base;
}
