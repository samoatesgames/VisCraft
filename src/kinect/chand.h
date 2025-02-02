#pragma once

#include <windows.h>
#include <NuiApi.h>
#include <SAM.h>
#include <d3dx9.h>
#include <time.h>
#include <sapi.h>

#include "../helper.h"
#include "CDeformableTemplateModel.h"
#include "gestures/CGestureHandClosed.h"

struct HandState {
	enum Enum {
		ClosedFist,
		OpenHand,
		NotFound,
		Noof
	};
};

struct ConfigurationState {
	enum Enum {
		None,
		Started,
		HasOpenHand,
		HasClosedHand,
		HasVoice,
		Configured,
		Noof
	};
};

class CHand {
private:

	unsigned int									m_frameWidth;									//!< 
	unsigned int									m_frameHeight;									//!<

	D3DXVECTOR2										m_palm;											//!<
	D3DXVECTOR2										m_lastPosition;									//!< 
	D3DXVECTOR2										m_oldPosition;									//!< 
	D3DXVECTOR2										m_center;
	D3DXVECTOR2										m_handSize;

	SAM::TVector<unsigned int, 4>					m_handArea;										//!< 
	HandState::Enum									m_handState;

	CDeformableTemplateModel						*m_handStateDTM[HandState::Noof];				//!< 	

	RGBQUAD											*m_edgeTempBuffer;								//!< 

	clock_t											m_startClose;									//!< 

	ConfigurationState::Enum						m_configuratState;								//!< The current state of configuration

private:

													//! Try to sample the data down to a smaller area,
													//! Where the hand could be located
	bool											SampleToHandArea(
														RGBQUAD *depthData
													);

													//! Detect the edges of the hands and display them in white
	void											DetectHandEdges(
														RGBQUAD *depthData
													);

													//! 
	void											DrawBox(
														RGBQUAD *depthData, 
														unsigned int x1, 
														unsigned int y1, 
														unsigned int x2, 
														unsigned int y2, 
														unsigned int r, 
														unsigned int g, 
														unsigned int b
													);

public:
													//! Class constructor
													CHand();

													//! Class destructor
													~CHand();

													//! Create the hand instance
	bool											Create(
														int frameWidth,
														int frameHeight
													);

													//! 
	void											Release();

													//! Try and find a hand from the depth image
	RGBQUAD*										FindFromDepth(
														RGBQUAD *depthData
													);

													//! Draw a green box around the sampled hand area for debugging
	void											DrawHandAreaBounds(
														RGBQUAD *depthData
													);

													//! Get the current hand position
	const D3DXVECTOR2								GetHandPosition();

													//! Get the current hand state
	HandState::Enum									GetHandState() const;

													//! 
	void											ResetHandPosition()
													{
														m_lastPosition = m_oldPosition;
													}
	
};