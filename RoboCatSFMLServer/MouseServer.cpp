#include "RoboCatServerPCH.hpp"


MouseServer::MouseServer()
{
}

void MouseServer::HandleDying()
{
	NetworkManagerServer::sInstance->UnregisterGameObject(this);
}


bool MouseServer::HandleCollisionWithCat(RoboCat* inCat)
{
	//calculate and apply damage to inflict to player depending on player velocity
	//float velocityFactor = 1.5 * (abs(inCat->GetVelocity().mX) + abs(inCat->GetVelocity().mY));
	//float sizeFactor = 0.1;//* GetRadius();
	//float damageToApply = 0.00005 * velocityFactor * sizeFactor;

	//((RoboCatServer*)inCat)->TakeDamage(damageToApply);

	//move the aircraft back 5 px in the opposite direction of normalized velocity
	Vector3 moveBack = inCat->GetVelocity();
	moveBack.mX = moveBack.mX / sqrt(pow(moveBack.mX, 2) + pow(moveBack.mY, 2)) * 5;
	moveBack.mY = moveBack.mY / sqrt(pow(moveBack.mX, 2) + pow(moveBack.mY, 2)) * 5;

	inCat->SetLocation(inCat->GetLocation() - moveBack);

	//apply bounced velocity
	inCat->SetVelocity(Vector3(inCat->GetVelocity().mX * -1, inCat->GetVelocity().mY * -1,0));

	return false;
}






