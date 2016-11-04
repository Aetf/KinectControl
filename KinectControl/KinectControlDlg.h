
// KinectControlDlg.h : 头文件
//

#pragma once
#include "SkeletalViewerDlg.h"
#include "SetKinectAngleDlg.h"


// CKinectControlDlg 对话框
class CKinectControlDlg : public CDialogEx
{
// 构造
public:
	CKinectControlDlg(CWnd* pParent = NULL);	// 标准构造函数
	~CKinectControlDlg(); // 析构函数

// 对话框数据
	enum { IDD = IDD_KINECTCONTROL_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg LRESULT OnKickIdle(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()


// 对话框相关内容
private:
	int                      MessageBoxResource( UINT nID );
	void                     SetBodyInfo();

	bool                     m_bShowSkeleton; // 是否显示骨骼图
	bool                     m_bShoeSetAngle; // 是否显示角度设置对话框
	bool                     m_bSensorReady;  // Kinect是否就绪
	//bool                     m_bTrackingModeChanged; // 追踪模式是否改变过
	bool                     m_bControlStateChanged; // 鼠标控制状态是否改变过


	CSkeletalViewerDlg       m_dlgSkel; // 骨骼图
	CSetKinectAngleDlg       m_dlgSetAngle; // 设置Kinect角度

// Nui相关内容
public:
	HRESULT      Nui_Init();
	void         Nui_UnInit();
	void         Nui_GotSkeletonAlert();
	void         Nui_Zero();
	void         UpdateTrackedSkeletonSelection(int mode);
	enum         _SV_TRACKED_SKELETONS
				{
					SV_TRACKED_SKELETONS_DEFAULT,
					SV_TRACKED_SKELETONS_NEAREST,
					SV_TRACKED_SKELETONS_STICKY,
				} SV_TRACKED_SKELETONS;

private:
	static DWORD WINAPI     Nui_ProcessThread(LPVOID pParam);
	DWORD WINAPI            Nui_ProcessThreadPriv();
	void                    UpdateTrackedSkeletons( const NUI_SKELETON_FRAME & skel );
	void                    UpdateSkeletonTrackingFlag(DWORD flag, bool earse);

	CWinThread*             m_pThNuiProcess;
	CEvent                  m_EvNuiProcessStop;

	CEvent                  m_EvNextSkeletonEvent;
	CEvent                  m_EvNextDepthFrameEvent;

	DWORD                   m_StickySkeletonId;
	int                     m_TrackingMode;
	DWORD                   m_SkeletonTrackingFlags;
// 输入控制相关内容
public:
	void         MyInputControl(NUI_SKELETON_FRAME * pSkelFrame);
	void         MySetOrigin(NUI_SKELETON_FRAME * pSkelFrame);
	void         MyMersureBody(NUI_SKELETON_FRAME * pSkelFrame);
	void         MySmooth(NUI_SKELETON_FRAME * pSkelFrame);

	float                    m_armLength;  // 单位：米
	float                    m_bodyHeight;
	float                    m_spineLength;
	float                    m_shoulderWidth;
	bool                     m_bReverseHand; // 是否反转用来控制的手（默认左手）
	POINT                    m_ControlOrg; // 设置的控制原点，在鼠标空间中（65535x65535）
	CSize                    m_SystemRes; // 系统屏幕分辨率，单位像素
	float                    m_Zdistinct; // 虚拟屏幕距离身体 单位m
	float                    m_virtualHeight; // 虚拟屏幕高
	float                    m_virtualWidth;  // 虚拟屏幕宽
	LARGE_INTEGER            m_LastInRectTime; // 上一次检测到手进入虚拟屏幕时的时间
	float                    m_VirtualX[2]; // 上一帧与本帧的手的位置
	float                    m_VirtualY[2];

	// 控制用标志
	bool                     m_bControl;   // 控制鼠标，本标志置true，停止置false
	bool                     m_bMeasureBody; // 如果开始测量身体数据，置true，结束后恢复false
private:
	void                     MyGetSkeletonData(NUI_SKELETON_FRAME *pSkelFrame, NUI_SKELETON_DATA ** ppSkelData);
	bool                     MyTestInDistinct(float toSpine, float virtualX, float virtualY);
	
	// 状态用标志
	bool                     m_bBodyMeasured; // 是否已经测量身体数据
	bool                     m_bWaitForDrag; // 是否在等待拖拽
	bool                     m_bDragging; // 是否正在拖拽
	bool                     m_bNoMove; // 手是否在原位
	bool                     m_bLeftDown; // 左键是否处于按下
	bool                     m_bMoveLocked; // 移动锁定状态

	HCURSOR                  m_hWaitForDragCur; // 等待拖拽的光标

public:
	afx_msg void OnClickedBtnMsrbdy();
	afx_msg void OnClickedBtnShowskel();
	afx_msg void OnClickedBtnStartctrl();
	afx_msg void OnClose();
//	afx_msg void OnClickedBtnChangeTrackingMode();
	afx_msg void OnClickedBtnActbanding();
//	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	CComboBox m_CmbTrackMode;
	afx_msg void OnSelchangeTrackmodeList();
	afx_msg void OnBnClickedBtnSettings();
};
