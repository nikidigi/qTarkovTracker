#ifndef PLAYERMARKER_H
#define PLAYERMARKER_H

#include "arrowmapmarker.h"

class PulseEffectItem;

class PlayerMapMarker : public ArrowMapMarker
{
public:
    explicit PlayerMapMarker(QGraphicsItem *parent = nullptr);

private:
    PulseEffectItem *m_pulse = nullptr;
};

#endif // PLAYERMARKER_H
