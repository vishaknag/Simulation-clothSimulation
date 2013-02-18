#include "vector.h"

point computeHooksForce(int indexA, int indexB, point collisionP, cloth *clothItem, int springType, int coEffType);

point computeDampingForce(int indexA, int indexB, point collisionP, cloth *clothItem, int coEffType);

void updateForce(node curP, node typeP, cloth *clothItem, int springType);

int checkIfInsideCloth(node A, cloth *clothItem);

void computeAcceleration(cloth *clothItem);

void checkForObjectCollision(node curP, cloth *clothItem);

void RK4(cloth *clothItem);

void Euler(cloth *clothItem);

void MoveClothXZ(cloth *clothItem, int direction);