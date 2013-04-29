/*
 * Implementation of the protected methods in the Drawer class.
 *
 * Author: Jeroen Baert
 */

#include "Drawer.h"

Drawer::Drawer(bool isvisible, bool iscompute ): visible_(isvisible),compute_(iscompute){

}
void Drawer::init(){

}
void Drawer::toggleVisibility(){
	visible_ = !visible_;
}

bool Drawer::isVisible(){
	return visible_;
}

void Drawer::setUnvisible(){
	visible_ = false;
}

bool Drawer::isCompute(){return this->compute_;}
void Drawer::setCompute(){compute_ = true;}
void Drawer::setUncompute(){compute_ = false;}