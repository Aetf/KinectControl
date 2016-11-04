#include "stdafx.h"
#include <NuiApi.h>
#include <cmath>
#include "KinectControlDlg.h"

// 常用函数结构实现

// vector实现
struct vector
{
	float x;
	float y;
	vector(const Vector4 & obj);
	float abs(const vector &obj);
	vector();
};
vector operator-(const vector & lhs, const vector & rhs)
{
	vector res;
	res.x = lhs.x - rhs.x;
	res.y = lhs.y - rhs.y;
	return res;
}
float operator*(const vector & lhs, const vector & rhs)
{
	float res;
	res = lhs.x * rhs.x + lhs.y * rhs.y;
	return res;
}
float abs(const vector & obj)
{
	float res;
	res = sqrt( obj.x * obj.x + obj.y* obj.y);
	return res;
}
vector::vector(const Vector4 & obj)
{
	x = obj.x;
	y = obj.z;

}
vector::vector()
	:x(0),y(0)
{
}
float operator-(const Vector4 & lhs, const Vector4 & rhs)
{
	float res = 0.0f;
	res = pow(lhs.x - rhs.x, 2) + pow(lhs.y - rhs.y, 2) + pow(lhs.z - rhs.z, 2);
	res = sqrt(res);
	return res;
}

// Vector4的操作符
Vector4 operator+ (const Vector4 & lhs, const Vector4 & rhs)
{
	Vector4 res;
	res.x = lhs.x + rhs.x;
	res.y = lhs.y + rhs.y;
	res.z = lhs.z + rhs.z;
	res.w = 1;

	return res;
}

Vector4 operator/ (const Vector4 & lhs, const int & rhs)
{
	Vector4 res;
	res.x = lhs.x / rhs;
	res.y = lhs.y / rhs;
	res.z = lhs.z / rhs;
	res.w = 1;

	return res;
}

// 常用函数
inline bool WITHIN(float src, float a, float b)
{
	return (src >= a) && (src <= b);
}
// 常量定义
enum _MOVEMENT{
	MOVEMENT_NO_INPUT,
	MOUSE_MOVE,
	MOUSE_DRAG,
	MOUSE_ENDDRAG,
	MOUSE_LEFTDOWN,
	MOUSE_LEFTUP
};
const float cShake = 0.005f; // 允许的抖动范围
const int MAXINPUT = 20; // 每帧允许的最大输入
const int MAXCOUNT = 5; // MySmooth函数平均值队列长度

void CKinectControlDlg::MyInputControl(NUI_SKELETON_FRAME * pSkelFrame)
{
	// 取得追踪到的骨骼
	NUI_SKELETON_DATA * pSkelData = NULL;
	MyGetSkeletonData(pSkelFrame, &pSkelData);

	MySmooth(pSkelFrame);

	Vector4 leftHand = pSkelData->SkeletonPositions[m_bReverseHand?NUI_SKELETON_POSITION_HAND_RIGHT:NUI_SKELETON_POSITION_HAND_LEFT];
	Vector4 rightHand = pSkelData->SkeletonPositions[m_bReverseHand?NUI_SKELETON_POSITION_HAND_LEFT: NUI_SKELETON_POSITION_HAND_RIGHT];
	Vector4 wrist = pSkelData->SkeletonPositions[m_bReverseHand?NUI_SKELETON_POSITION_WRIST_RIGHT:NUI_SKELETON_POSITION_WRIST_LEFT];
	Vector4 origin;

	int movement[MAXINPUT] = {MOVEMENT_NO_INPUT};
	int hasinput = 0;
	POINT mousePoint;

	// 初始化原点
	origin.y = pSkelData->SkeletonPositions[NUI_SKELETON_POSITION_HEAD].y + pSkelData->SkeletonPositions[NUI_SKELETON_POSITION_SHOULDER_CENTER].y;
	origin.y /= 2;
	origin.x = pSkelData->SkeletonPositions[m_bReverseHand?NUI_SKELETON_POSITION_SHOULDER_RIGHT: NUI_SKELETON_POSITION_SHOULDER_LEFT].x - m_shoulderWidth;
	origin.z = pSkelData->SkeletonPositions[m_bReverseHand? NUI_SKELETON_POSITION_SHOULDER_RIGHT: NUI_SKELETON_POSITION_SHOULDER_LEFT].z;

	float ltospine = pSkelData->SkeletonPositions[NUI_SKELETON_POSITION_SHOULDER_CENTER].z - leftHand.z;
	float rtospine = pSkelData->SkeletonPositions[NUI_SKELETON_POSITION_SHOULDER_CENTER].z - rightHand.z + pSkelData->SkeletonPositions[NUI_SKELETON_POSITION_HIP_CENTER].z - rightHand.z;
	rtospine /= 2;

	float virtualX = leftHand.x - origin.x;
	float virtualY = origin.y - leftHand.y;
	float virtualRX = rightHand.x - origin.x;
	float virtualRY = origin.y - rightHand.y;

	mousePoint.x = long(virtualX / m_virtualWidth * 65535);
	mousePoint.y = long(virtualY / m_virtualHeight * 65535);
	
	LARGE_INTEGER curInRectTime = pSkelFrame->liTimeStamp;
	if(MyTestInDistinct(ltospine,virtualX,virtualY))
	{
		// 移动判断
		if(virtualX >= 0 && virtualX <= m_virtualWidth
			&& virtualY >= 0 && virtualY <= m_virtualHeight)
		{
			if(!m_bMoveLocked)
				movement[hasinput++] = MOUSE_MOVE;
		}
		// 单击
		if(MyTestInDistinct(rtospine,virtualRX,virtualRY))
		{
			if(!m_bLeftDown)
			{
				movement[hasinput++] = MOUSE_LEFTDOWN;
				movement[hasinput++] = MOUSE_LEFTUP;
				m_bLeftDown = true;
				m_bMoveLocked = true;
			}
		}
		if(!MyTestInDistinct(rtospine,virtualRX,virtualRY) && m_bLeftDown)
		{
			m_bLeftDown = false;
			m_bMoveLocked = false;
		}
	}

	// 拖拽判断
	if(!MyTestInDistinct(ltospine,virtualX,virtualY))
	{
		if(m_bDragging)
		{
			m_bDragging = false;
			movement[hasinput++] = MOUSE_ENDDRAG;
		}
	}
	else
	{
		if(virtualX >= 0 && virtualX <= m_virtualWidth
						&& virtualY >= 0 && virtualY <= m_virtualHeight)
		{
			// 更新鼠标位置
			m_VirtualX[1] = m_VirtualX[0];
			m_VirtualY[1] = m_VirtualY[0];

			m_VirtualX[0] = virtualX;
			m_VirtualY[0] = virtualY;

			// 判断是否在原位
			float res = 0.0f;
			res = pow(m_VirtualX[0] - m_VirtualX[1], 2) + pow(m_VirtualY[0] - m_VirtualY[1], 2);
			res = sqrt(res);
			m_bNoMove = res < cShake;

			// 更新最后一次动的时间
			if(!m_bNoMove)
			{
				m_LastInRectTime = pSkelFrame->liTimeStamp;
			}
		}

		if((curInRectTime.QuadPart - m_LastInRectTime.QuadPart) > 1000)
		{
			// 拖拽等待
			if(!m_bDragging && m_bNoMove)
				m_bWaitForDrag = true;
		}

		if((curInRectTime.QuadPart - m_LastInRectTime.QuadPart) > 2000)
		{
			// 开始拖拽
			if(!m_bDragging && m_bWaitForDrag && m_bNoMove)
			{
				m_bWaitForDrag = false;
				m_bDragging = true;
				movement[hasinput++] = MOUSE_DRAG;
			}
		}
	}

#ifdef _DEBUG
			CString str;
			str.Format(L"%d %d %d",m_bNoMove,m_bDragging, m_bWaitForDrag);
			GetDlgItem(IDC_DEBUG2)->SetWindowText((LPCTSTR)str);
#endif
	// TODO: 其他判断


	INPUT input[MAXINPUT]={0};
	for(int i = 0; i != hasinput; i++)
	{
		
		input[i].mi.dx = mousePoint.x;
		input[i].mi.dy = mousePoint.y;
		input[i].mi.mouseData = 0;
		input[i].mi.dwExtraInfo =NULL;
		input[i].mi.time = 0;
		switch(movement[i])
		{
		case MOUSE_MOVE:
			input[i].type = INPUT_MOUSE;
			input[i].mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE;
			break;
		case MOUSE_DRAG:
		case MOUSE_LEFTDOWN:
			input[i].type = INPUT_MOUSE;
			input[i].mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_LEFTDOWN;
			break;
		case MOUSE_ENDDRAG:
		case MOUSE_LEFTUP:
			input[i].type = INPUT_MOUSE;
			input[i].mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_LEFTUP;
			break;
		}
	}
	if(hasinput)
		SendInput(hasinput, input, sizeof(input[0]));
}


void CKinectControlDlg::MyMersureBody(NUI_SKELETON_FRAME * pSkelFrame)
{
	// 取得追踪到的骨骼
	NUI_SKELETON_DATA * pSkelData = NULL;
	MyGetSkeletonData(pSkelFrame, &pSkelData);

	// TODO: 检测要用到的关节是否存在，若不存在发出提示，提示使用者调整位置或者调节Kinect的角度
	float arm[2], spine, shoulder;
	arm[0] = pSkelData->SkeletonPositions[NUI_SKELETON_POSITION_HAND_RIGHT] - pSkelData->SkeletonPositions[NUI_SKELETON_POSITION_SHOULDER_RIGHT];
	arm[1] = pSkelData->SkeletonPositions[NUI_SKELETON_POSITION_HAND_LEFT] - pSkelData->SkeletonPositions[NUI_SKELETON_POSITION_SHOULDER_LEFT];
	m_armLength = (arm[0] + arm[1]) / 2;

	/*body[0] = pSkelData->SkeletonPositions[NUI_SKELETON_POSITION_HEAD].y - pSkelData->SkeletonPositions[NUI_SKELETON_POSITION_FOOT_RIGHT].y;
	body[1] = pSkelData->SkeletonPositions[NUI_SKELETON_POSITION_HEAD].y - pSkelData->SkeletonPositions[NUI_SKELETON_POSITION_FOOT_LEFT].y;
	m_bodyHeight = (body[0] +body[1]) / 2;*/

	if(pSkelData->eSkeletonPositionTrackingState[NUI_SKELETON_POSITION_HIP_CENTER] == NUI_SKELETON_POSITION_NOT_TRACKED)
		TRACE("WARN! Hip Center not tracked!\n");
	spine = pSkelData->SkeletonPositions[NUI_SKELETON_POSITION_HEAD].y - pSkelData->SkeletonPositions[NUI_SKELETON_POSITION_HIP_CENTER].y;
	m_spineLength = spine;

	shoulder = pSkelData->SkeletonPositions[NUI_SKELETON_POSITION_SHOULDER_RIGHT].x - pSkelData->SkeletonPositions[NUI_SKELETON_POSITION_SHOULDER_LEFT].x;
	m_shoulderWidth = shoulder;

	SetBodyInfo();

	float Ytop, Ybottom, Xright, Xleft;
	m_Zdistinct = 0.4f* m_armLength;
	Ytop = (pSkelData->SkeletonPositions[NUI_SKELETON_POSITION_HEAD].y + pSkelData->SkeletonPositions[NUI_SKELETON_POSITION_SHOULDER_CENTER].y) / 2;
	Ybottom = pSkelData->SkeletonPositions[NUI_SKELETON_POSITION_HIP_CENTER].y;
	Xright = pSkelData->SkeletonPositions[NUI_SKELETON_POSITION_SHOULDER_RIGHT].x;
	Xleft = pSkelData->SkeletonPositions[NUI_SKELETON_POSITION_SHOULDER_LEFT].x - m_shoulderWidth;

	m_virtualHeight = Ytop - Ybottom;
	m_virtualWidth = Xright - Xleft;


	m_bMeasureBody = false;
	m_bBodyMeasured = true;
}

void CKinectControlDlg::MySmooth(NUI_SKELETON_FRAME * pSkelFrame)
{
	static Vector4 lefthand[MAXCOUNT] = {0};
	static Vector4 righthand[MAXCOUNT]= {0};
	static Vector4 head[MAXCOUNT] = {0};
	static Vector4 shouldercenter[MAXCOUNT] = {0};
	static Vector4 leftshoulder[MAXCOUNT] = {0};

	static int curCount = 0;
	
	NUI_SKELETON_DATA *pSkelData;
	MyGetSkeletonData(pSkelFrame, &pSkelData);

	NUI_TRANSFORM_SMOOTH_PARAMETERS smoothParams = 
	{0.5f, 0.5f, 0.5f, 0.05f, 0.04f};
	if(WITHIN(pSkelData->SkeletonPositions[m_bReverseHand?NUI_SKELETON_POSITION_HAND_RIGHT:NUI_SKELETON_POSITION_HAND_LEFT].x,
		pSkelData->SkeletonPositions[NUI_SKELETON_POSITION_SHOULDER_LEFT].x,
		pSkelData->SkeletonPositions[NUI_SKELETON_POSITION_SHOULDER_RIGHT].x))
	{
		smoothParams.fJitterRadius = 0.02f;
		smoothParams.fMaxDeviationRadius = 0.10f;
	}

	NuiTransformSmooth(pSkelFrame,&smoothParams);

	lefthand[curCount % MAXCOUNT] = pSkelData->SkeletonPositions[NUI_SKELETON_POSITION_HAND_LEFT];
	righthand[curCount % MAXCOUNT] = pSkelData->SkeletonPositions[NUI_SKELETON_POSITION_HAND_RIGHT];
	head[curCount % MAXCOUNT] = pSkelData->SkeletonPositions[NUI_SKELETON_POSITION_HEAD];
	shouldercenter[curCount % MAXCOUNT]=pSkelData->SkeletonPositions[NUI_SKELETON_POSITION_SHOULDER_CENTER];
	leftshoulder[curCount % MAXCOUNT]=pSkelData->SkeletonPositions[NUI_SKELETON_POSITION_SHOULDER_LEFT];
	
	curCount++;

	Vector4 sum[5] = {0};
	if(curCount < MAXCOUNT)
	{
		for(int i = 0; i != curCount; i++)
		{
			sum[0] = sum[0] + lefthand[i];
			sum[1] = sum[1] + righthand[i];
			sum[2] = sum[2] + head[i];
			sum[3] = sum[3] + shouldercenter[i];
			sum[4] = sum[4] + leftshoulder[i];
		}
		
		for(int i = 0; i != 5; i++)
			sum[i] = sum[i] / curCount;

		pSkelData->SkeletonPositions[NUI_SKELETON_POSITION_HAND_LEFT] = sum[0];
		pSkelData->SkeletonPositions[NUI_SKELETON_POSITION_HAND_RIGHT] = sum[1];
		pSkelData->SkeletonPositions[NUI_SKELETON_POSITION_HEAD] = sum[2];
		pSkelData->SkeletonPositions[NUI_SKELETON_POSITION_SHOULDER_CENTER] = sum[3];
		pSkelData->SkeletonPositions[NUI_SKELETON_POSITION_SHOULDER_LEFT] = sum[4];

	}
	else
	{
		for(int i = 0; i != MAXCOUNT; i++)
		{
			sum[0] = sum[0] + lefthand[i];
			sum[1] = sum[1] + righthand[i];
			sum[2] = sum[2] + head[i];
			sum[3] = sum[3] + shouldercenter[i];
			sum[4] = sum[4] + leftshoulder[i];
		}
		
		for(int i = 0; i != 5; i++)
			sum[i] = sum[i] / MAXCOUNT;

		pSkelData->SkeletonPositions[NUI_SKELETON_POSITION_HAND_LEFT] = sum[0];
		pSkelData->SkeletonPositions[NUI_SKELETON_POSITION_HAND_RIGHT] = sum[1];
		pSkelData->SkeletonPositions[NUI_SKELETON_POSITION_HEAD] = sum[2];
		pSkelData->SkeletonPositions[NUI_SKELETON_POSITION_SHOULDER_CENTER] = sum[3];
		pSkelData->SkeletonPositions[NUI_SKELETON_POSITION_SHOULDER_LEFT] = sum[4];
	}
}

void CKinectControlDlg::MyGetSkeletonData(NUI_SKELETON_FRAME * pSkelFrame, NUI_SKELETON_DATA ** ppSkelData)
{
	for(int i = 0; i != NUI_SKELETON_COUNT; i++)
	{
		if(pSkelFrame->SkeletonData[i].eTrackingState == NUI_SKELETON_TRACKED)
		{
			*ppSkelData = pSkelFrame->SkeletonData + i;
			break;
		}
	}
}

bool CKinectControlDlg::MyTestInDistinct(float toSpine, float virtualX, float virtualY)
{
	float distinct = 0.0f;
	float tmp = sqrt(pow(abs(virtualX - m_virtualWidth),2) + pow(abs(virtualY - m_virtualHeight),2));
	distinct = m_Zdistinct - (0.025f/(1.65f - (m_shoulderWidth / 1.5f) * tmp) - 0.025f);
	return toSpine >= distinct;
}
