#pragma once

#include "../cterrain.h"
#include "../cinput.h"


class IBrush {

protected:

	float											m_size;											//!< The size of the brush
	float											m_strength;										//!< The strength of the brush

public:
													//! Class constructor
													IBrush() : m_size(1.0f), m_strength(1.0f)
													{

													}

													//! Class destructor
	virtual											~IBrush() {};

													//! Apply the brush to the terrain
	virtual void									Apply(
														CInput *input,								//!< The input device being used for the brush
														CTerrain *terrain							//!< The terrain object we want to apply the brush too
													) = 0;


};