#include "actor_thread.hpp"
#include <Windows.h>

////////////////////////////////////////////////////
#define	NUM_LIFT	1
#define NUM_FLOOR	10
#define LIFT_SPEED	1	//0.5 floor/s

enum State {
	StateUp,
	StateDn,
	StateStop
};

struct LiftSensorMsg
{
	int lift;
	int floor;
	State dir;

	std::string str()
	{
		char s[100];
		sprintf(s, "LiftSensorMsg %d %d %d", lift, floor, dir);
		return std::string(s);
	}

	bool from(std::string const &str)
	{
		if (sscanf(str.c_str(), "LiftSensorMsg %d %d %d", &lift, &floor, &dir) > 0)
			return true;
	}
};

struct ClickInnerBtnMsg
{
	int lift;
	int floor;
	
	std::string str()
	{
		char s[100];
		sprintf(s, "ClickInnerBtnMsg %d %d", lift, floor);
		return std::string(s);
	}

	bool from(std::string const &str)
	{
		if (sscanf(str.c_str(), "ClickInnerBtnMsg %d %d", &lift, &floor)>0)
			return true;
	}
};

struct ClickOuterBtnMsg
{
	State dir;
	int floor;

	std::string str()
	{
		char s[100];
		sprintf(s, "ClickOuterBtnMsg %d %d", dir, floor);
		return std::string(s);
	}

	bool from(std::string const &str)
	{
		if (sscanf(str.c_str(), "ClickOuterBtnMsg %d %d", &dir, &floor)>0)
		return true;
	}
};

struct GetLiftFloorMsgRpy
{
	int floor;

	std::string str()
	{
		char s[100];
		sprintf(s, "GetLiftFloorMsg %d",floor);
		return std::string(s);
	}

	bool from(std::string const &str)
	{
		if (sscanf(str.c_str(), "GetLiftFloorMsg %d",  &floor)>0)
			return true;
	}
};

class Lift : public Actor
{
	double m_z;
	State m_state;
	bool m_bQuit;
	Address m_controller;
	int m_id;
	std::thread m_thLift;

public:

	Lift(Framework &frm, Address controller, int liftId) :Actor(frm), m_controller(controller), m_id(liftId)
	{
		m_z = 0;
		m_state = StateStop;
		m_bQuit = false;

		m_thLift = std::thread(&Lift::liftThd, this);
		RegisterHandler(std::bind(&Lift::run, this));
	}

	~Lift()
	{
		m_thLift.join();
	}
private:
	
	void liftThd()
	{
		while (!m_bQuit)
		{
			Sleep(1000);

			if (m_state!=StateStop)
			{
				//判断是否到达某个楼层了
				m_z += LIFT_SPEED*(m_state == StateUp ? 1 : -1);

				if (m_z <= 0 || m_z >= NUM_FLOOR - 1)
				{
					if (m_z <= 0)
						m_z = 0;
					if (m_z >= NUM_FLOOR - 1)
						m_z = NUM_FLOOR - 1;

					m_state = StateStop;
				}

				int floor = int(m_z);

				if (fabs(m_z - floor)<0.01) {

					LiftSensorMsg msg;
					msg.lift = m_id;
					msg.floor = floor;
					msg.dir = m_state;
					
					send(m_controller, msg.str());
				}

				
				
			}
			
		}
	}
	void run()
	{
		while (true)
		{
			QueueItem i = receive();
			//printf("%s\n", i.msg.c_str());
			//send(i.sender, i.msg);

			GetLiftFloorMsgRpy mGetLiftFloorMsgRpy;

			if (i.msg == "EXIT")
			{
				return;
			}
			else if (i.msg == "Up")
			{
				m_state = StateUp;
			}
			else if (i.msg == "Down")
			{
				m_state = StateDn;
			}
			else if (i.msg == "Stop")
			{
				m_state = StateStop;
			}
			else if (mGetLiftFloorMsgRpy.from(i.msg))
			{
				mGetLiftFloorMsgRpy.floor = (int)m_z;
				send(i.sender, mGetLiftFloorMsgRpy.str());
			}
		}
	}
};

class Controller : public Actor
{
	int m_innerBtns[NUM_LIFT][NUM_FLOOR];
	int m_outerBtns[NUM_FLOOR][2];
	int m_liftFloor[NUM_LIFT];
	State m_liftDir[NUM_LIFT];
	Address m_lifts[NUM_LIFT];
	std::thread m_th1;
public:

	Controller(Framework &frm) :Actor(frm)
	{
		memset(m_innerBtns, 0, sizeof(m_innerBtns));
		memset(m_outerBtns, 0, sizeof(m_outerBtns));
		memset(m_liftFloor, 0, sizeof(m_liftFloor));
		std::fill(m_liftDir, m_liftDir+NUM_LIFT, StateStop);
		memset(m_lifts, 0, sizeof(m_lifts));

		RegisterHandler(std::bind(&Controller::run, this));
	}

	void setLift(int lift, Address addr)
	{
		m_lifts[lift] = addr;
	}
	void print()
	{
		system("cls");

		for (size_t i = 0; i < NUM_FLOOR; i++)
		{
			int floor = NUM_FLOOR - i - 1;
			printf("%2d ", floor);

			for (size_t j = 0; j < NUM_LIFT; j++)
			{
				if (m_liftFloor[j]== floor)
				{
					int dir = m_liftDir[j];
					char *sDir = "^v-";
					printf("%c ", sDir[dir]);
				}
				else
				{
					printf("  ");
				}
			}
			printf("\n");
		}
	}
private:
	int getLiftCurrFloor(int lift)
	{
		return m_liftFloor[lift];
	}

	void clickInnerFloor(int lift, int floor, int click=1)
	{
		m_innerBtns[lift][floor] = click;
	}
	
	bool isInnerFloorClicked(int lift, int floor)
	{
		return m_innerBtns[lift][floor] == 1;
	}
	void clickOutBtn(int floor, bool isUp, int click = 1)
	{
		m_outerBtns[floor][isUp ? 0 : 1] = click;
	}

	bool isOutBtnClicked(int floor, bool isUp)
	{
		return m_outerBtns[floor][isUp ? 0 : 1];
	}

	void run()
	{
		while (true)
		{
			QueueItem i = receive();
			//printf("%s\n", i.msg.c_str());
			//send(i.sender, i.msg);

			LiftSensorMsg mLiftSensorMsg;
			ClickInnerBtnMsg mClickInnerBtnMsg;
			ClickOuterBtnMsg mClickOuterBtnMsg;
			GetLiftFloorMsgRpy mGetLiftFloorMsgRpy;

			if (i.msg == "EXIT")
			{
				return;
			}
			else if (mLiftSensorMsg.from(i.msg))
			{
				m_liftFloor[mLiftSensorMsg.lift] = mLiftSensorMsg.floor;
				m_liftDir[mLiftSensorMsg.lift] = mLiftSensorMsg.dir;

				//如果电梯到达某层，可能需要开门
				if (isInnerFloorClicked(mLiftSensorMsg.lift, mLiftSensorMsg.floor)	//如果里面目标楼层有着一层
					|| isOutBtnClicked(mLiftSensorMsg.floor, mLiftSensorMsg.dir==StateUp)	//如果外面这一层有按下，并且方向一致
					|| (isOutBtnClicked(mLiftSensorMsg.floor, mLiftSensorMsg.dir != StateUp) &&		//如果外面这一层有按下，并且方向相反，而且里面没人按下
						std::find(m_innerBtns[mLiftSensorMsg.lift], m_innerBtns[mLiftSensorMsg.lift]+NUM_FLOOR,1)== m_innerBtns[mLiftSensorMsg.lift] + NUM_FLOOR)
					)
				{
					clickInnerFloor(mLiftSensorMsg.lift, mLiftSensorMsg.floor, 0);
					clickOutBtn(mLiftSensorMsg.floor, mLiftSensorMsg.dir == StateUp, 0);

					Address sender = i.sender;
					State dir = mLiftSensorMsg.dir;
					m_th1 = std::thread([this, sender, dir, mLiftSensorMsg]() {
						send(sender, std::string("Stop"));
						Sleep(2000);

						//如果里面的按钮没人按下，则停下来
						if(std::find(m_innerBtns[mLiftSensorMsg.lift], m_innerBtns[mLiftSensorMsg.lift] + NUM_FLOOR, 1) != m_innerBtns[mLiftSensorMsg.lift] + NUM_FLOOR)
							send(sender, std::string(dir== StateUp?"Up":"Down"));
					});
				}
			}
			else if (mClickOuterBtnMsg.from(i.msg))
			{
				//设置变量状态
				clickOutBtn(mClickOuterBtnMsg.floor, mClickOuterBtnMsg.dir == StateUp);
				//看看有没有空闲的电梯，如有，则向该方向移动
				for (size_t i = 0; i < NUM_LIFT; i++)
				{
					if (m_liftDir[i] == StateStop)
					{
						send(m_lifts[i], std::string(m_liftFloor[i]<mClickOuterBtnMsg.floor ? "Up" : "Down"));
						break;
					}
				}
			}
			else if (mClickInnerBtnMsg.from(i.msg))
			{
				//设置变量状态
				clickInnerFloor(mClickInnerBtnMsg.lift, mClickInnerBtnMsg.floor);
			}
		}
	}
};

class person : public Actor
{
public:

	person(Framework &frm) :Actor(frm)
	{
		RegisterHandler(std::bind(&person::run, this));
	}

private:

	void run()
	{
		
	}
};

void main() {
	Framework frm;
	
	Controller c(frm);
	Lift *s[NUM_LIFT];
	person p(frm);

	for (size_t i = 0; i < NUM_LIFT; i++)
	{
		s[i] = new Lift(frm, c.addr(), i);
		c.setLift(i, s[i]->addr());
	}
	
	std::thread thdPrint([&]() {
		while (1) {
			c.print(); Sleep(500);
		}
	});
	frm.Send(p.addr(), c.addr(), ClickOuterBtnMsg{ StateDn, 5 }.str());
	
	system("pause");
}
