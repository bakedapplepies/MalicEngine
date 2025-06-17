#pragma once

#include "Engine/Malic.h"

namespace MalicClient
{

void ProcessInput(const Malic::MalicEngine* engine, float delta_time);
void MoveCamera(const Malic::MalicEngine* engine, float delta_time);
void RotateCamera(const Malic::MalicEngine* engine, float delta_time);

}  // namespace MalicClient