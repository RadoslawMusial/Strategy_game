#ifndef JEDNOSTKI_H
#define JEDNOSTKI_H

typedef struct {
  int stamina;
  int speed;
  int value;
  int attack_range;
  int build_time;
} Unit;

void Knight(Unit *K);
void Swordsman(Unit *S);
void Archer(Unit *A);
void Pikeman(Unit *P);
void Ram(Unit *R);
void Catapult(Unit *C);
void Worker(Unit *W);
void Base(Unit *B);

#endif
