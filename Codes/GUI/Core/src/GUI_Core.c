#include "GUI_Core.h"
#include "GUI.h"

void *GUI_Heap[2];           /* �ڴ��ָ�� */
GUI_HWIN GUI_RootWin;        /* ������ */
GUI_AREA GUI_AreaHeap;       /* �ü������ */
GUI_CONTEXT GUI_Context;     /* GUI������ */
static u_32 __LockTaskId;
static u_16 __TaskLockCnt;

/* GUI��ʼ�� */
GUI_RESULT GUI_Init(void)
{
    u_32 HeapSize;

    /* �ڴ������ʼ�� */
    GUI_Heap[GUI_HEAP_FAST] = _GUI_GetHeapBuffer(GUI_HEAP_FAST, &HeapSize);
    if (GUI_HeapInit(GUI_Heap[GUI_HEAP_FAST], HeapSize) == GUI_ERR) {
        return GUI_ERR;
    }
    GUI_Heap[GUI_HEAP_HCAP] = _GUI_GetHeapBuffer(GUI_HEAP_HCAP, &HeapSize);
    if (GUI_HeapInit(GUI_Heap[GUI_HEAP_HCAP], HeapSize) == GUI_ERR) {
        return GUI_ERR;
    }
    /* ��ʼ������ϵͳ��ش��� */
    GUI_InitOS();
    /* ��ʼ��ͼ��Ӳ�� */
    GUI_DeviceInit();
    /* ��ʼ�����ڼ�����ü�˽�ж� */
    if (GUI_RectListInit(GUI_RECT_HEAP_SIZE) == GUI_ERR) {
        return GUI_ERR;
    }
    /* ��ʼ����Ϣ���� */
    if (GUI_MessageQueueInit() == GUI_ERR) {
        return GUI_ERR;
    }
    /* ��ʼ�����ڹ����� */
    if (WM_Init() == GUI_ERR) {
        return GUI_ERR;
    }
    GUI_SetFont(&GUI_DEF_FONT);
    return GUI_OK;
}

/* ���ڴ���ж��GUI */
void GUI_Unload(void)
{
    GUI_LOCK();
    WM_DeleteWindow(_hRootWin); /* ɾ�����д��� */
    GUI_MessageQueueDelete();   /* ɾ����Ϣ���� */
    GUI_UNLOCK();
}

/* ��ȡ��Ļ�ߴ� */
void GUI_ScreenSize(u_16 *xSize, u_16 *ySize)
{
    *xSize = GUI_LCD.xSize;
    *ySize = GUI_LCD.ySize;
}

/* ��ȡ��Ļ���� */
u_16 GUI_GetScreenWidth(void)
{
    return GUI_LCD.xSize;
}

/* ��ȡ��Ļ�߶� */
u_16 GUI_GetScreenHeight(void)
{
    return GUI_LCD.ySize;
}

/* GUI��ʱ������ */
void GUI_Delay(GUI_TIME tms)
{
    GUI_TIME t = GUI_GetTime();

    WM_Exec();
    t = GUI_GetTime() - t; /* ����ִ��WM_Exec()��ʱ�� */
    if (tms > t) {
        _GUI_Delay_ms(tms - t); /* ��ʱ */
    }
}

/* -------------------- GUI������ -------------------- */

/* GUI���� */
void GUI_LOCK(void)
{
    /* ��û�����������Ѿ��������������� */
    if (!__TaskLockCnt || __LockTaskId != GUI_GetTaskId()) {
        GUI_TaskLock();
        __LockTaskId = GUI_GetTaskId();
    }
    ++__TaskLockCnt;
}

/* GUI���� */
void GUI_UNLOCK(void)
{
    if (--__TaskLockCnt == 0) {
        GUI_TaskUnlock();
    }
}

/* -------------------- GUI���μ��� -------------------- */
/* �������ڻ�������Ĳü��������� */
void GUI_SetNowRectList(GUI_AREA l, GUI_RECT *p)
{
    GUI_Context.Area = l;
    GUI_Context.InvalidRect = p;
}

/* �������ڻ�������Ĳü��������� */
GUI_AREA GUI_GetNowRectList(void)
{
    return GUI_Context.Area;
}

/* ��ʼ���������� */
void GUI_DrawAreaInit(GUI_RECT *p)
{
    if (GUI_RectOverlay(&GUI_Context.DrawRect, GUI_Context.InvalidRect, p)) {
        GUI_Context.pAreaNode = GUI_Context.Area;
    } else {
        GUI_Context.pAreaNode = NULL; /* ��ͼ�����뵱ǰ����Ч���������ཻ */
    }
}

/* ��ȡ��һ���ü����� */
GUI_BOOL GUI_GetNextArea(void)
{
    GUI_BOOL res = FALSE;
    GUI_AREA Area;
    GUI_RECT *ClipRect = &GUI_Context.ClipRect,
             *DrawRect = &GUI_Context.DrawRect;

    while (GUI_Context.pAreaNode && res == FALSE) { /* ֱ���ҵ���һ���ཻ�ľ��� */
        Area = GUI_Context.pAreaNode;
        GUI_Context.pAreaNode = Area->pNext;
        res = GUI_RectOverlay(ClipRect, DrawRect, &Area->Rect);
    }
    return res;
}

/* -------------------- GUI���������� -------------------- */
/* ���õ�ǰ���� */
void GUI_SetFont(GUI_FONT *Font)
{
    GUI_Context.Font = Font;
}

/* ���ñ���ɫ */
void GUI_SetBackgroundColor(GUI_COLOR Color)
{
    GUI_Context.BGColor = Color;
}

/* ����ǰ��ɫ */
void GUI_SetForegroundColor(GUI_COLOR Color)
{
    GUI_Context.FGColor = Color;
}

/* ����������ɫ */
void GUI_SetFontColor(GUI_COLOR Color)
{
    GUI_Context.FontColor = Color;
}

/* GUI������� */
#if GUI_DEBUG_MODE
void GUI_DebugOut(const char *s)
{
    _GUI_DebugOut(s);
}
#endif