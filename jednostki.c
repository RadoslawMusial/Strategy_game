#include "jednostki.h"

void Knight(Unit *K) {
  K->stamina = 70;
  K->speed = 5;
  K->value = 400;
  K->attack_range = 1;
  K->build_time = 5;
}

void Swordsman(Unit *S) {
  S->stamina = 60;
  S->speed = 2;
  S->value = 250;
  S->attack_range = 1;
  S->build_time = 5;
}

void Archer(Unit *A) {
  A->stamina = 40;
  A->speed = 2;
  A->value = 250;
  A->attack_range = 5;
  A->build_time = 3;
}

void Pikeman(Unit *P) {
  P->stamina = 50;
  P->speed = 2;
  P->value = 200;
  P->attack_range = 2;
  P->build_time = 3;
}

void Ram(Unit *R) {
  R->stamina = 90;
  R->speed = 2;
  R->value = 500;
  R->attack_range = 1;
  R->build_time = 4;
}

void Catapult(Unit *C) {
  C->stamina = 50;
  C->speed = 2;
  C->value = 800;
  C->attack_range = 7;
  C->build_time = 6;
}

void Worker(Unit *W) {
  W->stamina = 20;
  W->speed = 2;
  W->value = 100;
  W->attack_range = 1;
  W->build_time = 2;
}

void Base(Unit *B) {
  B->stamina = 200;
  B->speed = 0;
}
