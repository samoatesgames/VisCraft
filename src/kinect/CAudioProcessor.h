#pragma  once

#include <sapi.h>
#include "../CGui.h"

class CAudioProcessor {

private:
	struct AudioPhrases {
		enum Enum {
			VisCraft,
			ExitApplication,
			Cancel,
			Brushes,
			BrushRaise,
			BrushLower,
			BrushDeform,
			File,
			FileNew,
			FileOpen,
			FileSave,
			About,
			Noof
		};
	};
private:

	bool									m_saidViscraft;
	CGui									*m_gui;

public:
											CAudioProcessor(
												CGui *gui	
											);

											~CAudioProcessor();

	void									Process(
												const SPPHRASEPROPERTY* semantic
											);

};