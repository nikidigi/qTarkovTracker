#include "playermapmarker.h"
#include "pulseeffectitem.h"

PlayerMapMarker::PlayerMapMarker(QGraphicsItem *parent)
    : ArrowMapMarker(parent)
{
    m_pulse = new PulseEffectItem(this);
    m_pulse->start();
}