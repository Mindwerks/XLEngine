#ifndef INPUT_H
#define INPUT_H

#include "VirtualKeys.h"
#include <vector>
using std::vector;

typedef void (*Input_KeyDownCB)(int32_t);

//static input class
//the Windowing system (Win API on Windows XP/Vista/7, Cocoa on OS X) passes the keyboard and mouse
//messages to this class, which is then used by the game systems.
class Input
{
public:
	//init and destroy, done by the OS layer.
	static void Init();
	static void Destroy();
	//called from the OS layer.
	static void SetKeyDown(int32_t key);
	static void SetKeyUp(int32_t key);
	static void SetCharacterDown(char c);
	static void SetMousePos(f32 x, f32 y);
	static void ClearAllKeys();
	//can be called by any game system.
	inline static bool IsKeyDown(int32_t key) { return m_aKeyState[key] ? true : false; }
	inline static f32 GetMouseX() { return m_fMouseX; }
	inline static f32 GetMouseY() { return m_fMouseY; }
	inline static f32 GetMouseDx() { return m_fMouseDeltaX; }
	inline static f32 GetMouseDy() { return m_fMouseDeltaY; }
	//Mouse
	inline static XL_BOOL LockMouse() { return m_bLockMouse; }
	static void EnableMouseLocking(XL_BOOL bEnable);
	inline static void SetMouseDelta(int32_t dx, int32_t dy) { m_fMouseDeltaX = (f32)dx; m_fMouseDeltaY = (f32)dy; }
	inline static void AddMouseDelta(int32_t dx, int32_t dy) { m_fMouseDeltaX += (f32)dx; m_fMouseDeltaY += (f32)dy; }
	inline static void GetMouseDelta(f32& dx, f32& dy) { dx = m_fMouseDeltaX; dy = m_fMouseDeltaY; }

	//setup an event callback. Some systems can get key down events - useful for edit boxes and general text editing.
	//this allows systems to be setup that don't use polling and that are somewhat frame rate independent (depending on the OS).
	static bool AddKeyDownCallback( Input_KeyDownCB pCB, int32_t nFlags=KDCb_FLAGS_NONE );
	static bool AddCharDownCallback( Input_KeyDownCB pCB );
public:
	enum
	{
		KDCb_FLAGS_NONE = 0,
		KDCb_FLAGS_NOREPEAT = 1,
	} KeyDownCB_Flags_e;
private:
	typedef struct
	{
		Input_KeyDownCB pCB;
		int32_t nFlags;
	} KeyDownCB_t;

	static int8_t m_aKeyState[512];
	static vector<KeyDownCB_t *> m_KeyDownCB;
	static vector<KeyDownCB_t *> m_CharDownCB;
	static f32 m_fMouseX;
	static f32 m_fMouseY;
	static f32 m_fMouseDeltaX;
	static f32 m_fMouseDeltaY;
	static XL_BOOL m_bLockMouse;
};

#endif	//INPUT_H
