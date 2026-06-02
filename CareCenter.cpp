#include "CareCenter.h"

CareCenter::CareCenter(int id, double x, double y, int capacity)
    : m_id(id), m_x(x), m_y(y), m_capacity(capacity) {}

int CareCenter::GetId() const { return m_id; }
double CareCenter::GetX() const { return m_x; }
double CareCenter::GetY() const { return m_y; }
int CareCenter::GetCapacity() const { return m_capacity; }