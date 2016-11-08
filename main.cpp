/*
* Edited By Jet, weaZen, Moriarty
* 2016/11/07 23:55
*/
//这是一个爱射的版本
/*
* 相较处男变化：
* 1. 增加DisDir结构，用于同时返回distance和
* direct。避免了原先繁杂的位运算。
* GameField::GetTo同一位置时返回
* atPos(0, stay)， 达不到返回 naPos(-1, noPos)。
* 
* 2. 废掉了distance储存的功能，因为慢而且有bug。
* 
* 3. 增加GameField::DirValid，用于判断往某
* 方向走是否会撞墙（不能传stay）。
*
* 4. 增加GameField::canEat，用于判断玩家在dir
* 方向是否能立刻吃到豆子。
*
* 5. 增加:
*   bool GameField::canDggbHit(int myID, int rivalID)
*       判断myID是否能立刻射到rivalID（假设对方不动）
*   bool GameField::canDggbHit(Direction dir, int myID, int rivalID)
*       判断myID往dir走后是否能射到rivalID（假设rivalID不动）
*       （写注释的时候，我意识到了这个函数的问题）
*   bool GameField::canDggbHitR(Direction dir, int myID, int rivalID)
*       判断rivalID往dir走后是否能射到myID（假设myID不动）
*   调用上述任何一个函数，若返回true时，会更新gameField.hitDir的值
*   用于传回可以射的方向。
* 
* 6. 增加Helpers::MY_ID全局记录我的ID。
* 
* 7. SimpleSearch中大果子不止加一分，
* 而是加gameField.largeFruitValue分。
*
* 8. 搜索部分统统没改，因为搜索加入shoot方向后，很容易让AI很中二的想多
* 一加就乱走，射和躲射部分都写在chooseDir里了（核心改动）。
*
*/

#include <fstream>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <iostream>
#include <algorithm>
#include <string>
#include <stack>
#include <stdexcept>
#include <vector>
#include <set>
#include "jsoncpp/json.h"

#define FIELD_MAX_HEIGHT    20
#define FIELD_MAX_WIDTH     20
#define MAX_GENERATOR_COUNT 4 // 每个象限1
#define MAX_PLAYER_COUNT    4
#define MAX_TURN            100
#define TIME_LIMIT          0.97
#define QUEUE_MAX           121
#define MAX_INT             0x3fffffff
#define DEFAULT_DEPTH       1
#define MAX_DEPTH           15
#define DEATH_EVAL          -1000000
#define INVALID_EVAL        -9999999

//#define DEBUG
//#define PROFILING
//#define PRESET

// 你也可以选用 using namespace std; 但是会污染命名空间
using std::cin;
using std::cout;
using std::endl;
using std::swap;
using std::string;
using std::getline;
using std::to_string;
using std::runtime_error;

// 用于调试
namespace Debug
{
    auto printInfo = false;
    string presetString =
#ifdef PRESET
            R"({"requests":[{"GENERATOR_INTERVAL":20,"LARGE_FRUIT_DURATION":10,"LARGE_FRUIT_ENHANCEMENT":10,"SKILL_COST":4,"content":[[16,0,16,16,0,16,16,0,16],[16,16,16,16,0,16,16,16,16],[16,0,16,0,0,0,16,0,16],[16,1,32,0,0,0,32,2,16],[0,0,0,0,0,0,0,0,0],[0,0,0,0,0,0,0,0,0],[16,4,32,0,0,0,32,8,16],[16,0,16,0,0,0,16,0,16],[16,16,16,16,0,16,16,16,16],[16,0,16,16,0,16,16,0,16]],"height":10,"id":3,"seed":1478532798,"static":[[8,5,1,5,5,5,1,5,2],[0,7,12,5,5,5,6,13,0],[10,31,9,5,1,5,3,31,10],[2,13,4,3,10,9,4,7,8],[12,5,5,2,14,8,5,5,6],[9,5,5,2,11,8,5,5,3],[2,13,1,6,10,12,1,7,8],[10,31,12,5,4,5,6,31,10],[0,7,9,5,5,5,3,13,0],[8,5,4,5,5,5,4,5,2]],"width":9},{"0":{"action":1},"1":{"action":3},"2":{"action":1},"3":{"action":3}},{"0":{"action":0},"1":{"action":3},"2":{"action":2},"3":{"action":3}},{"0":{"action":-1},"1":{"action":2},"2":{"action":0},"3":{"action":0}},{"0":{"action":-1},"1":{"action":1},"2":{"action":1},"3":{"action":1}},{"0":{"action":1},"1":{"action":1},"2":{"action":0},"3":{"action":1}},{"0":{"action":1},"1":{"action":1},"2":{"action":3},"3":{"action":1}},{"0":{"action":1},"1":{"action":0},"2":{"action":-1},"3":{"action":2}},{"0":{"action":1},"1":{"action":1},"2":{"action":3},"3":{"action":1}},{"0":{"action":2},"1":{"action":3},"2":{"action":1},"3":{"action":2}},{"0":{"action":3},"1":{"action":0},"2":{"action":3},"3":{"action":2}},{"0":{"action":2},"1":{"action":0},"2":{"action":-1},"3":{"action":2}},{"0":{"action":2},"1":{"action":0},"2":{"action":-1},"3":{"action":2}},{"0":{"action":2},"1":{"action":0},"2":{"action":-1},"3":{"action":2}},{"0":{"action":1},"1":{"action":3},"2":{"action":1},"3":{"action":1}},{"0":{"action":2},"1":{"action":3},"2":{"action":-1},"3":{"action":3}},{"0":{"action":-1},"1":{"action":0},"2":{"action":-1},"3":{"action":2}},{"0":{"action":-1},"1":{"action":3},"2":{"action":3},"3":{"action":0}},{"0":{"action":-1},"1":{"action":3},"2":{"action":-1},"3":{"action":3}},{"0":{"action":-1},"1":{"action":3},"2":{"action":3},"3":{"action":3}},{"0":{"action":-1},"1":{"action":3},"2":{"action":2},"3":{"action":-1}},{"0":{"action":0},"1":{"action":2},"2":{"action":2},"3":{"action":1}},{"0":{"action":1},"1":{"action":3},"2":{"action":2},"3":{"action":2}},{"0":{"action":3},"1":{"action":1},"2":{"action":1},"3":{"action":2}},{"0":{"action":3},"1":{"action":3},"2":{"action":-1},"3":{"action":1}},{"0":{"action":0},"1":{"action":1},"2":{"action":-1},"3":{"action":0}},{"0":{"action":0},"1":{"action":1},"2":{"action":-1},"3":{"action":0}},{"0":{"action":0},"1":{"action":1},"2":{"action":3},"3":{"action":0}},{"0":{"action":1},"1":{"action":1},"2":{"action":3},"3":{"action":1}},{"0":{"action":1},"1":{"action":1},"2":{"action":0},"3":{"action":1}},{"0":{"action":3},"1":{"action":0},"2":{"action":0},"3":{"action":2}},{"0":{"action":0},"1":{"action":2},"2":{"action":2},"3":{"action":1}},{"0":{"action":3},"1":{"action":3},"2":{"action":0},"3":{"action":1}},{"0":{"action":3},"1":{"action":3},"2":{"action":2},"3":{"action":1}},{"0":{"action":3},"1":{"action":3},"2":{"action":2},"3":{"action":1}},{"0":{"action":3},"1":{"action":3},"2":{"action":3},"3":{"action":0}},{"0":{"action":2},"1":{"action":3},"2":{"action":1},"3":{"action":1}},{"0":{"action":3},"1":{"action":3},"2":{"action":3},"3":{"action":1}},{"0":{"action":1},"1":{"action":2},"2":{"action":-1},"3":{"action":2}},{"0":{"action":1},"1":{"action":0},"2":{"action":-1},"3":{"action":3}},{"0":{"action":2},"1":{"action":0},"2":{"action":-1},"3":{"action":-1}},{"0":{"action":0},"1":{"action":0},"2":{"action":-1},"3":{"action":1}},{"0":{"action":3},"1":{"action":0},"2":{"action":-1},"3":{"action":2}},{"0":{"action":3},"1":{"action":3},"2":{"action":-1},"3":{"action":2}},{"0":{"action":1},"1":{"action":2},"2":{"action":-1},"3":{"action":1}},{"0":{"action":0},"1":{"action":2},"2":{"action":-1},"3":{"action":0}},{"0":{"action":1},"1":{"action":2},"2":{"action":5},"3":{"action":0}},{"0":{"action":1},"1":{"action":2},"2":{"action":5},"3":{"action":1}},{"0":{"action":1},"1":{"action":3},"2":{"action":1},"3":{"action":3}},{"0":{"action":1},"1":{"action":3},"2":{"action":1},"3":{"action":0}},{"0":{"action":2},"1":{"action":2},"2":{"action":3},"3":{"action":1}},{"0":{"action":1},"1":{"action":3},"2":{"action":1},"3":{"action":1}},{"0":{"action":3},"1":{"action":3},"2":{"action":1},"3":{"action":1}},{"0":{"action":3},"1":{"action":3},"2":{"action":3},"3":{"action":3}},{"0":{"action":2},"1":{"action":1},"2":{"action":2},"3":{"action":2}}],"responses":[{"action":3,"tauntText":""},{"action":3,"tauntText":""},{"action":0,"tauntText":""},{"action":1,"tauntText":""},{"action":1,"tauntText":""},{"action":1,"tauntText":""},{"action":2,"tauntText":""},{"action":1,"tauntText":""},{"action":2,"tauntText":""},{"action":2,"tauntText":""},{"action":2,"tauntText":""},{"action":2,"tauntText":""},{"action":2,"tauntText":""},{"action":1,"tauntText":""},{"action":3,"tauntText":""},{"action":2,"tauntText":""},{"action":0,"tauntText":""},{"action":3,"tauntText":""},{"action":3,"tauntText":""},{"action":-1,"tauntText":""},{"action":1,"tauntText":""},{"action":2,"tauntText":""},{"action":2,"tauntText":""},{"action":1,"tauntText":""},{"action":0,"tauntText":""},{"action":0,"tauntText":""},{"action":0,"tauntText":""},{"action":1,"tauntText":""},{"action":1,"tauntText":""},{"action":2,"tauntText":""},{"action":1,"tauntText":""},{"action":1,"tauntText":""},{"action":1,"tauntText":""},{"action":1,"tauntText":""},{"action":0,"tauntText":""},{"action":1,"tauntText":""},{"action":1,"tauntText":""},{"action":2,"tauntText":""},{"action":3,"tauntText":""},{"action":-1,"tauntText":""},{"action":1,"tauntText":""},{"action":2,"tauntText":""},{"action":2,"tauntText":""},{"action":1,"tauntText":""},{"action":0,"tauntText":""},{"action":0,"tauntText":""},{"action":1,"tauntText":""},{"action":3,"tauntText":""},{"action":0,"tauntText":""},{"action":1,"tauntText":""},{"action":1,"tauntText":""},{"action":1,"tauntText":""},{"action":3,"tauntText":""},{"action":2,"tauntText":""}]})";
#else
            "";
#endif


    Json::Value debugData;
    auto timeOutFlag = false;
    auto startTime = clock();

    inline double TimeThrough(clock_t stt = startTime)
    {
        return double(clock() - stt) / CLOCKS_PER_SEC;
    }

    inline bool TimeOut()
    {
#ifdef DEBUG
        return false;
#endif
        if (timeOutFlag)
            return true;
        if (TimeThrough() > TIME_LIMIT)
        {
            debugData["profiling"]["TimeOut"] = true;
            return timeOutFlag = true;
        }
        return false;
    }
}

//把枚举扩展收起来
namespace EnumExt
{
// 让枚举也可以用这些运算了（不加会编译错误）
    template<typename T>
    inline T operator|=(T& a, const T& b)
    {
        return a = static_cast<T>(static_cast<int>(a) | static_cast<int>(b));
    }

    template<typename T>
    inline T operator|(const T& a, const T& b)
    {
        return static_cast<T>(static_cast<int>(a) | static_cast<int>(b));
    }

    template<typename T>
    inline T operator&=(T& a, const T& b)
    {
        return a = static_cast<T>(static_cast<int>(a) & static_cast<int>(b));
    }

    template<typename T>
    inline T operator&(const T& a, const T& b)
    {
        return static_cast<T>(static_cast<int>(a) & static_cast<int>(b));
    }

    template<typename T>
    inline T operator-(const T& a, const T& b)
    {
        return static_cast<T>(static_cast<int>(a) - static_cast<int>(b));
    }

    template<typename T>
    inline T operator++(T& a)
    {
        return a = static_cast<T>(static_cast<int>(a) + 1);
    }

    template<typename T>
    inline T operator~(const T& a)
    {
        return static_cast<T>(~static_cast<int>(a));
    }
}

// 平台提供的吃豆人相关逻辑处理程序
namespace Pacman
{
    using namespace EnumExt;

    const auto seed = time(nullptr);
    const int dx[] = {0, 1, 0, -1, 1, 1, -1, -1}, dy[] = {-1, 0, 1, 0, -1, 1, 1, -1};
    const string dirStr[] = {"stay", "up", "right", "down", "left", "shootUp", "shootRight", "shootDown", "shootLeft"};

// 枚举定义；使用枚举虽然会浪费空间（sizeof(GridContentType) == 4），但是计算机处理32位的数字效率更高

// 每个格子可能变化的内容，会采用“或”逻辑进行组合
    enum GridContentType
    {
        player1 = 1,                // 1号玩家
        player2 = 2,                // 2号玩家
        player3 = 4,                // 3号玩家
        player4 = 8,                // 4号玩家
        playerMask = 1 | 2 | 4 | 8, // 用于检查有没有玩家等
        smallFruit = 16,            // 小豆子
        largeFruit = 32             // 大豆子
    };

// 用玩家ID换取格子上玩家的二进制位
    GridContentType playerID2Mask[] = {player1, player2, player3, player4};
    string playerID2str[] = {"0", "1", "2", "3"};

// 每个格子固定的东西，会采用“或”逻辑进行组合
    enum GridStaticType
    {
        wallNorth = 1, // 北墙（纵坐标减少的方向）
        wallEast = 2,  // 东墙（横坐标增加的方向）
        wallSouth = 4, // 南墙（纵坐标增加的方向）
        wallWest = 8,  // 西墙（横坐标减少的方向）
        generator = 16 // 豆子产生器
    };

// 用移动方向换取这个方向上阻挡着的墙的二进制位
    GridStaticType direction2OpposingWall[] = {wallNorth, wallEast, wallSouth, wallWest};

// 方向，可以代入dx、dy数组，同时也可以作为玩家的动作
    enum Direction
    {
        stay = -1,
        up = 0,
        right = 1,
        down = 2,
        left = 3,
        shootUp = 4,    // 右上
        shootRight = 5, // 右下
        shootDown = 6,  // 左下
        shootLeft = 7,  // 左上
        // Jet
                noPos = 8
    };

// 场地上带有坐标的物件
    struct FieldProp
    {
        int row, col;

        FieldProp(int i = 0, int j = 0) : row(i), col(j) {}

        virtual bool operator==(const FieldProp& a) const { return (row == a.row && col == a.col); }

        virtual bool operator!=(const FieldProp& a) const { return (row != a.row || col != a.col); }

        virtual bool operator>(const FieldProp& a) const { return (row == a.row ? col > a.col : row > a.row); }

        virtual bool operator<(const FieldProp& a) const { return (row == a.row ? col < a.col : row < a.row); }
    };

    struct PathInfoType : FieldProp
    {
        bool isImpasse;
        bool isExit;
        int fleeLength;   //到死路出口的距离
        int impasseDepth; //死路最大深度
        PathInfoType *pExit;

        PathInfoType(int y = 0, int x = 0) : FieldProp(y, x), isImpasse(false), isExit(false), fleeLength(0),
                                             impasseDepth(0), pExit(nullptr) {}
    };

    struct GenInfoType : FieldProp
    {
        bool isBesideGen;
        int fruitClusterCount;

        GenInfoType(int y = 0, int x = 0) : FieldProp(y, x), isBesideGen(false), fruitClusterCount(0) {}
    };

// 场地上的玩家
    struct Player : FieldProp
    {
        int strength;
        int powerUpLeft;
        bool dead;
    };

// 回合新产生的豆子的坐标
    struct
    {
        FieldProp newFruits[MAX_GENERATOR_COUNT * 8];
        int newFruitCount;
    }newFruits[MAX_TURN];
    int newFruitsCount = 0;

// 状态转移记录结构
    struct TurnStateTransfer
    {
        enum StatusChange // 可组合
        {
            none = 0,
            ateSmall = 1,
            ateLarge = 2,
            powerUpDrop = 4,
            die = 8,
            error = 16
        };

        // 玩家选定的动作
        Direction actions[MAX_PLAYER_COUNT];

        // 此回合该玩家的状态变化
        StatusChange change[MAX_PLAYER_COUNT];

        // 此回合该玩家的力量变化
        int strengthDelta[MAX_PLAYER_COUNT];
    };

// Jet: DisDir
    struct DisDir
    {
        unsigned char dis;
        Direction dir;

        explicit DisDir(int dist = 0, Direction dire = stay) : dis(dist), dir(dire) {}
    };

    auto atPos = DisDir(0, stay);
    auto naPos = DisDir(-1, Direction::noPos);

// 游戏主要逻辑处理类，包括输入输出、回合演算、状态转移，全局唯一
    class GameField
    {
    private:
        // 为了方便，大多数属性都不是private的

        // 记录每回合的变化（栈）
        TurnStateTransfer backtrack[MAX_TURN];

        // 这个对象是否已经创建
        static bool constructed;

    public:
        // 场地的长和宽
        int height, width;
        int generatorCount;
        int GENERATOR_INTERVAL, LARGE_FRUIT_DURATION, LARGE_FRUIT_ENHANCEMENT, SKILL_COST;

        //道路信息
        PathInfoType pathInfo[FIELD_MAX_HEIGHT][FIELD_MAX_WIDTH];
        GenInfoType genInfo[FIELD_MAX_HEIGHT][FIELD_MAX_WIDTH];
        int maxCluster;

        // 场地格子固定的内容
        GridStaticType fieldStatic[FIELD_MAX_HEIGHT][FIELD_MAX_WIDTH];

        // 场地格子会变化的内容
        GridContentType fieldContent[FIELD_MAX_HEIGHT][FIELD_MAX_WIDTH];
        int generatorTurnLeft; // 多少回合后产生豆子
        int aliveCount;        // 有多少玩家存活
        int smallFruitCount;
        int turnID;
        FieldProp generators[MAX_GENERATOR_COUNT]; // 有哪些豆子产生器
        Player players[MAX_PLAYER_COUNT];          // 有哪些玩家
        // 玩家选定的动作
        Direction actions[MAX_PLAYER_COUNT];

        bool hasNext; // weaZen：省得每次查一遍
        int largeFruitValue;
        char distance[FIELD_MAX_HEIGHT][FIELD_MAX_WIDTH][FIELD_MAX_HEIGHT][FIELD_MAX_WIDTH]{};

        // 恢复到上次场地状态。可以一路恢复到最开始。
        // 恢复失败（没有状态可恢复）返回false
        bool PopState()
        {
            hasNext = true;
            if (turnID <= 0)
                return false;

            const auto& bt = backtrack[--turnID];

            // 倒着来恢复状态
            for (int playerID = 0; playerID < MAX_PLAYER_COUNT; playerID++)
            {
                auto& player = players[playerID];
                auto& content = fieldContent[player.row][player.col];
                auto change = bt.change[playerID];

                // 5. 大豆回合恢复
                if (change & TurnStateTransfer::powerUpDrop)
                    player.powerUpLeft++;

                // 4. 吐出豆子
                if (change & TurnStateTransfer::ateSmall)
                {
                    content |= smallFruit;
                    smallFruitCount++;
                } else if (change & TurnStateTransfer::ateLarge)
                {
                    content |= largeFruit;
                    player.powerUpLeft -= LARGE_FRUIT_DURATION;
                }

                // 2. 魂兮归来
                if (change & TurnStateTransfer::die)
                {
                    player.dead = false;
                    aliveCount++;
                    content |= playerID2Mask[playerID];
                }

                // 1. 移形换影
                if (!player.dead && bt.actions[playerID] != stay && bt.actions[playerID] < shootUp)
                {
                    fieldContent[player.row][player.col] &= ~playerID2Mask[playerID];
                    player.row = (player.row - dy[bt.actions[playerID]] + height) % height;
                    player.col = (player.col - dx[bt.actions[playerID]] + width) % width;
                    fieldContent[player.row][player.col] |= playerID2Mask[playerID];
                }

                // 0. 救赎不合法的灵魂
                if (change & TurnStateTransfer::error)
                {
                    player.dead = false;
                    aliveCount++;
                    content |= playerID2Mask[playerID];
                }

                // *. 恢复力量
                player.strength -= bt.strengthDelta[playerID];
            }

            // 3. 收回豆子
            if (generatorTurnLeft == GENERATOR_INTERVAL)
            {
                generatorTurnLeft = 1;
                auto& fruits = newFruits[--newFruitsCount];
                for (int i = 0; i < fruits.newFruitCount; i++)
                {
                    fieldContent[fruits.newFruits[i].row][fruits.newFruits[i].col] &= ~smallFruit;
                    smallFruitCount--;
                }
            } else
                generatorTurnLeft++;

            return true;
        }

        // Jet:把PopState包装了一下 方便一些
        void RollBack(int turnCount = -1)
        {
#ifdef PROFILING
            auto &&startTime = clock();
#endif
            if (turnCount < 0)
                while (PopState());
            else
                for (int i = 0; i < turnCount; i++)
                    if (!PopState())
                        break;
#ifdef PROFILING
            auto &d = Debug::debugData["profiling"]["RollBack()"];
        d = d.asDouble() + double(clock() - startTime) / CLOCKS_PER_SEC;
#endif
        }

        // 判断指定玩家向指定方向移动是不是合法的（没有撞墙且没有踩到豆子产生器）
        inline bool ActionValid(int playerID, const Direction& dir) const
        {
            if (dir == stay)
                return true;
            const auto& p = players[playerID];
            if (dir >= shootUp)
                return dir < 8 && p.strength > SKILL_COST;
            return dir >= 0 && dir < 4 &&
                   !(fieldStatic[p.row][p.col] & direction2OpposingWall[dir]);
        }

        inline bool DirValid(FieldProp& p, const Direction& dir) const
        {
            return !(fieldStatic[p.row][p.col] & direction2OpposingWall[dir]);
        }

        // 在向actions写入玩家动作后，演算下一回合局面，并记录之前所有的场地状态，可供日后恢复。
        // 是终局的话就返回false
        bool NextTurn()
        {
#ifdef PROFILING
            auto &&startTime = clock();
#endif
            auto& bt = backtrack[turnID];
            memset(&bt, 0, sizeof bt);

            // 0. 杀死不合法输入
            for (int playerID = 0; playerID < MAX_PLAYER_COUNT; playerID++)
            {
                auto& player = players[playerID];
                if (!player.dead)
                {
                    auto& action = actions[playerID];
                    if (action == stay)
                        continue;

                    if (!ActionValid(playerID, action))
                    {
                        bt.strengthDelta[playerID] += -player.strength;
                        bt.change[playerID] = TurnStateTransfer::error;
                        fieldContent[player.row][player.col] &= ~playerID2Mask[playerID];
                        player.strength = 0;
                        player.dead = true;
                        aliveCount--;
                    } else if (action < shootUp)
                    {
                        // 遇到比自己强♂壮的玩家是不能前进的
                        const auto& target = fieldContent
                        [(player.row + dy[action] + height) % height]
                        [(player.col + dx[action] + width) % width];
                        if (target & playerMask)
                            for (int i = 0; i < MAX_PLAYER_COUNT; i++)
                                if (target & playerID2Mask[i] && players[i].strength > player.strength)
                                    action = stay;
                    }
                }
            }

            // 1. 位置变化
            for (int playerID = 0; playerID < MAX_PLAYER_COUNT; playerID++)
            {
                auto& player = players[playerID];

                bt.actions[playerID] = actions[playerID];

                if (player.dead || actions[playerID] == stay || actions[playerID] >= shootUp)
                    continue;

                // 移动
                fieldContent[player.row][player.col] &= ~playerID2Mask[playerID];
                player.row = (player.row + dy[actions[playerID]] + height) % height;
                player.col = (player.col + dx[actions[playerID]] + width) % width;
                fieldContent[player.row][player.col] |= playerID2Mask[playerID];
            }

            // 2. 玩家互殴
            for (int playerID = 0; playerID < MAX_PLAYER_COUNT; playerID++)
            {
                auto& player = players[playerID];
                if (player.dead)
                    continue;

                // 判断是否有玩家在一起
                int containedCount = 0;
                int containedPlayers[MAX_PLAYER_COUNT];
                for (int i = 0; i < MAX_PLAYER_COUNT; i++)
                    if (fieldContent[player.row][player.col] & playerID2Mask[i])
                        containedPlayers[containedCount++] = i;

                if (containedCount > 1)
                {
                    // NAIVE
                    for (int i = 0; i < containedCount; i++)
                        for (int j = 0; j < containedCount - i - 1; j++)
                            if (players[containedPlayers[j]].strength < players[containedPlayers[j + 1]].strength)
                                swap(containedPlayers[j], containedPlayers[j + 1]);

                    int begin;
                    for (begin = 1; begin < containedCount; begin++)
                        if (players[containedPlayers[begin - 1]].strength > players[containedPlayers[begin]].strength)
                            break;

                    // 这些玩家将会被杀死
                    int lootedStrength = 0;
                    for (int i = begin; i < containedCount; i++)
                    {
                        int id = containedPlayers[i];
                        Player& p = players[id];

                        // 从格子上移走
                        fieldContent[p.row][p.col] &= ~playerID2Mask[id];
                        p.dead = true;
                        int drop = p.strength / 2;
                        bt.strengthDelta[id] += -drop;
                        bt.change[id] |= TurnStateTransfer::die;
                        lootedStrength += drop;
                        p.strength -= drop;
                        aliveCount--;
                    }

                    // 分配给其他玩家
                    auto inc = lootedStrength / begin;
                    for (int i = 0; i < begin; i++)
                    {
                        auto id = containedPlayers[i];
                        auto& p = players[id];
                        bt.strengthDelta[id] += inc;
                        p.strength += inc;
                    }
                }
            }

            // 2.5 金光法器
            for (int playerID = 0; playerID < MAX_PLAYER_COUNT; playerID++)
            {
                Player& player = players[playerID];
                if (player.dead || actions[playerID] < shootUp)
                    continue;

                player.strength -= SKILL_COST;
                bt.strengthDelta[playerID] -= SKILL_COST;

                int r = player.row, c = player.col;
                Direction dir = actions[playerID] - shootUp;

                // 向指定方向发射金光（扫描格子直到被挡）
                while (!(fieldStatic[r][c] & direction2OpposingWall[dir]))
                {
                    r = (r + dy[dir] + height) % height;
                    c = (c + dx[dir] + width) % width;

                    // 如果转了一圈回来……
                    if (r == player.row && c == player.col)
                        break;

                    if (fieldContent[r][c] & playerMask)
                        for (int playerID2 = 0; playerID2 < MAX_PLAYER_COUNT; playerID2++)
                            if (fieldContent[r][c] & playerID2Mask[playerID2])
                            {
                                players[playerID2].strength -= SKILL_COST * 1.5;
                                bt.strengthDelta[playerID2] -= SKILL_COST * 1.5;
                                player.strength += SKILL_COST * 1.5;
                                bt.strengthDelta[playerID] += SKILL_COST * 1.5;
                            }
                }
            }

            // *. 检查一遍有无死亡玩家
            for (int playerID = 0; playerID < MAX_PLAYER_COUNT; playerID++)
            {
                Player& player = players[playerID];
                if (player.dead || player.strength > 0)
                    continue;

                // 从格子上移走
                fieldContent[player.row][player.col] &= ~playerID2Mask[playerID];
                player.dead = true;

                // 使其力量变为0
                bt.strengthDelta[playerID] += -player.strength;
                bt.change[playerID] |= TurnStateTransfer::die;
                player.strength = 0;
                aliveCount--;
            }

            // 3. 产生豆子
            if (--generatorTurnLeft == 0)
            {
                generatorTurnLeft = GENERATOR_INTERVAL;
                auto& fruits = newFruits[newFruitsCount++];
                fruits.newFruitCount = 0;
                for (int i = 0; i < generatorCount; i++)
                    for (auto d = up; d < 8; ++d)
                    {
                        // 取余，穿过场地边界
                        int r = (generators[i].row + dy[d] + height) % height, c =
                                (generators[i].col + dx[d] + width) % width;
                        if (fieldStatic[r][c] & generator || fieldContent[r][c] & (smallFruit | largeFruit))
                            continue;
                        fieldContent[r][c] |= smallFruit;
                        fruits.newFruits[fruits.newFruitCount].row = r;
                        fruits.newFruits[fruits.newFruitCount++].col = c;
                        smallFruitCount++;
                    }
            }

            // 4. 吃掉豆子
            for (int playerID = 0; playerID < MAX_PLAYER_COUNT; playerID++)
            {
                auto& player = players[playerID];
                if (player.dead)
                    continue;

                auto& content = fieldContent[player.row][player.col];

                // 只有在格子上只有自己的时候才能吃掉豆子
                if (content & playerMask & ~playerID2Mask[playerID])
                    continue;

                if (content & smallFruit)
                {
                    content &= ~smallFruit;
                    player.strength++;
                    bt.strengthDelta[playerID]++;
                    smallFruitCount--;
                    bt.change[playerID] |= TurnStateTransfer::ateSmall;
                } else if (content & largeFruit)
                {
                    content &= ~largeFruit;
                    if (player.powerUpLeft == 0)
                    {
                        player.strength += LARGE_FRUIT_ENHANCEMENT;
                        bt.strengthDelta[playerID] += LARGE_FRUIT_ENHANCEMENT;
                    }
                    player.powerUpLeft += LARGE_FRUIT_DURATION;
                    bt.change[playerID] |= TurnStateTransfer::ateLarge;
                }
            }

            // 5. 大豆回合减少
            for (int playerID = 0; playerID < MAX_PLAYER_COUNT; playerID++)
            {
                Player& player = players[playerID];
                if (player.dead)
                    continue;

                if (player.powerUpLeft > 0)
                {
                    bt.change[playerID] |= TurnStateTransfer::powerUpDrop;
                    if (--player.powerUpLeft == 0)
                    {
                        player.strength -= LARGE_FRUIT_ENHANCEMENT;
                        bt.strengthDelta[playerID] += -LARGE_FRUIT_ENHANCEMENT;
                    }
                }
            }

            // *. 检查一遍有无死亡玩家
            for (int playerID = 0; playerID < MAX_PLAYER_COUNT; playerID++)
            {
                Player& player = players[playerID];
                if (player.dead || player.strength > 0)
                    continue;

                // 从格子上移走
                fieldContent[player.row][player.col] &= ~playerID2Mask[playerID];
                player.dead = true;

                // 使其力量变为0
                bt.strengthDelta[playerID] += -player.strength;
                bt.change[playerID] |= TurnStateTransfer::die;
                player.strength = 0;
                aliveCount--;
            }

            ++turnID;
#ifdef PROFILING
            auto &&d = Debug::debugData["profiling"]["NextTurn()"];
        d = d.asDouble() + double(clock() - startTime) / CLOCKS_PER_SEC;
#endif
            // 是否只剩一人？
            if (aliveCount <= 1)
            {
                for (int playerID = 0; playerID < MAX_PLAYER_COUNT; playerID++)
                    if (!players[playerID].dead)
                    {
                        bt.strengthDelta[playerID] += smallFruitCount;
                        players[playerID].strength += smallFruitCount;
                    }
                return hasNext = false;
            }

            // 是否回合超限？
            if (turnID >= 100)
                return hasNext = false;
            return hasNext = true;
        }

        inline bool canEat(int playerID, Direction dir)
        {
            int r = (players[playerID].row + dy[dir] + height) % height;
            int c = (players[playerID].col + dx[dir] + width) % width;
            auto& content = fieldContent[r][c];
            return (content & smallFruit) || (content & largeFruit);
        }

        // weaZen: 双向
        // Jet: 用cc的改的
        char step[FIELD_MAX_HEIGHT][FIELD_MAX_WIDTH];

        char Distance(const FieldProp& startPos, const FieldProp& endPos)
        {
#ifdef PROFILING
            auto startTime = clock();
#endif
            if (startPos == endPos)
                return distance[startPos.row][startPos.col][endPos.row][endPos.col] = 0;

            if (distance[startPos.row][startPos.col][endPos.row][endPos.col])
                return distance[startPos.row][startPos.col][endPos.row][endPos.col];

            if (distance[endPos.row][endPos.col][startPos.row][startPos.col])
                return distance[startPos.row][startPos.col][endPos.row][endPos.col] = distance[endPos.row][endPos.col][startPos.row][startPos.col];

            //对死路的优化
            if (pathInfo[startPos.row][startPos.col].isImpasse && !pathInfo[endPos.row][endPos.col].isImpasse)
                return distance[startPos.row][startPos.col][endPos.row][endPos.col] =
                               Distance(*pathInfo[startPos.row][startPos.col].pExit, endPos) +
                               pathInfo[startPos.row][startPos.col].fleeLength;

            if (!pathInfo[startPos.row][startPos.col].isImpasse && pathInfo[endPos.row][endPos.col].isImpasse)
                return distance[startPos.row][startPos.col][endPos.row][endPos.col] =
                               Distance(startPos, *pathInfo[endPos.row][endPos.col].pExit) +
                               pathInfo[endPos.row][endPos.col].fleeLength;

            if (pathInfo[startPos.row][startPos.col].isImpasse && pathInfo[startPos.row][startPos.col].isImpasse &&
                pathInfo[startPos.row][startPos.col].pExit != pathInfo[startPos.row][startPos.col].pExit)
                return distance[startPos.row][startPos.col][endPos.row][endPos.col] =
                               Distance(*pathInfo[startPos.row][startPos.col].pExit,
                                        *pathInfo[endPos.row][endPos.col].pExit) +
                               pathInfo[endPos.row][endPos.col].fleeLength +
                               pathInfo[startPos.row][startPos.col].fleeLength;

            //初始化广搜数组
            memset(step, 0, FIELD_MAX_HEIGHT * FIELD_MAX_WIDTH * sizeof(char));

            step[startPos.row][startPos.col] = 1;
            step[endPos.row][endPos.col] = -1;

            //初始化广搜队列
            Pacman::FieldProp queue[QUEUE_MAX];
            queue[0] = startPos;
            queue[1] = endPos;
            auto nowFlag = 0, endFlag = 1;
            auto hasFound = false;
            auto ret = 0;

            while (nowFlag <= endFlag && !hasFound)
            {
                const auto& curGrid = fieldStatic[queue[nowFlag].row][queue[nowFlag].col];
                for (auto dir = Pacman::up; dir < 4; ++dir)
                {
                    if (!(curGrid & Pacman::direction2OpposingWall[dir]))
                    {
                        auto newPos = queue[nowFlag];
                        newPos.row = (newPos.row + Pacman::dy[dir] + height) % height;
                        newPos.col = (newPos.col + Pacman::dx[dir] + width) % width;
                        if (step[queue[nowFlag].row][queue[nowFlag].col] > 0)
                        {
                            if (step[newPos.row][newPos.col] > step[queue[nowFlag].row][queue[nowFlag].col] + 1 ||
                                step[newPos.row][newPos.col] == 0) //新的点是好的
                            {
                                step[newPos.row][newPos.col] = step[queue[nowFlag].row][queue[nowFlag].col] + 1;
                                distance[startPos.row][startPos.col][newPos.row][newPos.col] =
                                        step[newPos.row][newPos.col] - 1;
                                queue[++endFlag] = newPos;
                            }
                            if (step[newPos.row][newPos.col] < 0)
                            {
                                hasFound = true;
                                ret = step[queue[nowFlag].row][queue[nowFlag].col] - step[newPos.row][newPos.col] - 1;
                            }
                        }
                        if (step[queue[nowFlag].row][queue[nowFlag].col] < 0)
                        {
                            if (step[newPos.row][newPos.col] < step[queue[nowFlag].row][queue[nowFlag].col] - 1 ||
                                step[newPos.row][newPos.col] == 0) //新的点是好的
                            {
                                step[newPos.row][newPos.col] = step[queue[nowFlag].row][queue[nowFlag].col] - 1;
                                distance[endPos.row][endPos.col][newPos.row][newPos.col] =
                                        -step[newPos.row][newPos.col] - 1;
                                queue[++endFlag] = newPos;
                            }
                            if (step[newPos.row][newPos.col] > 0)
                            {
                                hasFound = true;
                                ret = -step[queue[nowFlag].row][queue[nowFlag].col] + step[newPos.row][newPos.col] - 1;
                            }
                        }
                    }
                }
                ++nowFlag;
            }

#ifdef PROFILING
            auto &&d = Debug::debugData["profiling"]["Distance()"];
        d = d.asDouble() + double(clock() - startTime) / CLOCKS_PER_SEC;
#endif
            return distance[startPos.row][startPos.col][endPos.row][endPos.col] = ret;
        }

        // Moriartycc: 牢记位运算优先级
        int Distance(int alphaID, int betaID)
        {
            return Distance(players[alphaID], players[betaID]);
        }

        // weaZen: 返回distance<<3 + dir + 1以便决策
        // Jet: 改写了个模版
        Direction dirInfo[FIELD_MAX_HEIGHT][FIELD_MAX_WIDTH];

        template<typename __Pred>
        DisDir GetTo(int myID, __Pred pr, char forbiddenDirs = '\0')
        {
#ifdef PROFILING
            auto &&startTime = clock();
#endif

            auto startPos = players[myID];
            if (pr(*this, startPos) && !(forbiddenDirs & 1))
                return atPos;

            std::vector<std::pair<FieldProp, int>> targetList;

            int initMinDis = MAX_INT;
            int minDis = MAX_INT;
            int tryDis[4] = {MAX_INT, MAX_INT, MAX_INT, MAX_INT};

            for (int i = 0; i < height; i++)
                for (int j = 0; j < width; j++)
                {
                    if (i == startPos.row && j == startPos.col)
                        continue;
                    if (pr(*this, FieldProp(i, j)))
                    {
                        int tmp = int(Distance(startPos, FieldProp(i, j)));
                        if (tmp >= initMinDis + 2)
                            continue;
                        targetList.push_back(std::make_pair(FieldProp(i, j), tmp));
                        initMinDis = std::min(initMinDis, tmp);
                    }
                }

            if (targetList.empty())
                return naPos;

            for (auto i = targetList.begin(); i != targetList.end();)
            {
                if ((*i).second >= initMinDis + 2)
                    i = targetList.erase(i);
                else
                    ++i;
            }

            for (auto d = up; d <= left; ++d)
            {
                if (fieldStatic[startPos.row][startPos.col] & direction2OpposingWall[d])
                    continue;
                FieldProp checkPos((startPos.row + dy[d] + height) % height, (startPos.col + dx[d] + width) % width);
                for (auto i = targetList.begin(); i != targetList.end(); ++i)
                    tryDis[d] = std::min(tryDis[d], int(Distance(checkPos, (*i).first)));
                minDis = std::min(minDis, tryDis[d]);
            }

            int tmp = 1;
            auto tmpDir = stay;
            for (auto d = up; d <= left; ++d)
                if (tryDis[d] == minDis && !(forbiddenDirs & (1 << (d + 1))))
                    tmpDir = tmpDir == stay ? d : ((rand() % ++tmp) ? tmpDir : d);

            if (tmpDir != stay)
            {
#ifdef PROFILING
                auto &&d = Debug::debugData["profiling"]["GetTo()"];
            d = d.asDouble() + double(clock() - startTime) / CLOCKS_PER_SEC;
#endif
                return DisDir(minDis, tmpDir);
            }

            //初始化广搜数组
            for (int i = 0; i < height; i++)
                for (int j = 0; j < width; j++)
                    dirInfo[i][j] = Pacman::stay;

            memset(step, 31, FIELD_MAX_HEIGHT * FIELD_MAX_WIDTH * sizeof(char));
            step[startPos.row][startPos.col] = 0;

            //初始化广搜队列
            FieldProp queue[QUEUE_MAX];
            queue[0] = startPos;
            char nowFlag = 0, endFlag = 0;
            char dis = 0;
            bool hasEaten = false;

            //初始化随机方向
            Direction randomDir[4];
            for (int i = 0; i < 4; ++i)
                randomDir[i] = Direction(i);

            //禁止的方向设为已经访问
            for (int i = 0; i < 4; ++i)
                if ((forbiddenDirs & (1 << (i + 1))) &&
                    !(fieldStatic[startPos.row][startPos.col] & direction2OpposingWall[i]))
                    dirInfo[(startPos.row + dy[i] + height) % height][(startPos.col + dx[i] + width) % width] = up;

            while (nowFlag <= endFlag && !hasEaten)
            {
                const auto& curGrid = fieldStatic[queue[nowFlag].row][queue[nowFlag].col];
                for (int i = 0; i < 4; ++i)
                    swap(randomDir[rand() % 4], randomDir[rand() % 4]);
                Direction dir;
                for (int i = 0; i < 4; ++i)
                {
                    dir = randomDir[i];
                    if (!(curGrid & direction2OpposingWall[dir]))
                    {
                        auto newPos = queue[nowFlag];
                        newPos.row = (newPos.row + dy[dir] + height) % height;
                        newPos.col = (newPos.col + dx[dir] + width) % width;
                        if (dirInfo[newPos.row][newPos.col] == -1 && newPos != startPos)
                        {
                            if (step[newPos.row][newPos.col] > step[queue[nowFlag].row][queue[nowFlag].col] + 1)
                                step[newPos.row][newPos.col] = step[queue[nowFlag].row][queue[nowFlag].col] + 1;
                            dirInfo[newPos.row][newPos.col] = dir;
                            queue[++endFlag] = newPos;
                            if (pr(*this, newPos))
                            {
                                hasEaten = true;
                                dis = step[queue[endFlag].row][queue[endFlag].col];
                                break;
                            }
                        }
                    }
                }
                ++nowFlag;
            }
            if (!hasEaten)
                return naPos;

            //回溯
            auto dir = stay;
            auto curPos = queue[endFlag];
            while (curPos != startPos)
            {
                dir = dirInfo[curPos.row][curPos.col];
                curPos.row = (curPos.row - dy[dir] + height) % height;
                curPos.col = (curPos.col - dx[dir] + width) % width;
            }

#ifdef PROFILING
            auto &&d = Debug::debugData["profiling"]["GetTo()"];
        d = d.asDouble() + double(clock() - startTime) / CLOCKS_PER_SEC;
#endif

            return DisDir(dis, dir);
        }

        //weaZen:照着cc的广搜写了个寻找方向 target是GridContentType里的组合 可以试一下吃人了//ω\\)
        DisDir GetToTarget(int myID, int target, char forbiddenDirs = '\0')
        {
            if (target == 0)
                return naPos;
            return GetTo(myID,
                         ([target](const GameField& gameField, const FieldProp& pos)
                         {
                             return gameField.fieldContent[pos.row][pos.col] & target;
                         }),
                         forbiddenDirs);
        }

        bool atMaxCluster(const FieldProp& pos)
        {
            return genInfo[pos.row][pos.col].fruitClusterCount == maxCluster;
        }

        DisDir GetToNearbyGenerator(int myID, char forbiddenDirs = '\0')
        {
            return GetTo(myID, [](const GameField& gameField, const FieldProp& pos)
                         {
                             return gameField.genInfo[pos.row][pos.col].isBesideGen;
                         },
                         forbiddenDirs);
        }

        DisDir GetToMaxCluster(int myID, char forbiddenDirs = '\0')
        {
            return GetTo(myID, [](const GameField& gameField, const FieldProp& pos)
                         {
                             return (gameField.genInfo[pos.row][pos.col].fruitClusterCount == gameField.maxCluster);
                         },
                         forbiddenDirs);
        }


        Direction hitDir;

        bool canDggbHit(int myID, int rivalID)
        {
#ifdef PROFILING
            auto &&startTime = clock();
#endif
            const auto& me = players[myID], rival = players[rivalID];
            const int myR = me.row, myC = me.col;
            const int riR = rival.row, riC = rival.col;
            if (myC == riC)
            {
                if (myR == riR)
                    return false;
                int r = myR;
                while (!(fieldStatic[r][myC] & direction2OpposingWall[down]))
                {
                    r = (r + 1 + height) % height;
                    if (r == myR)
                    {
#ifdef PROFILING
                        auto &&d = Debug::debugData["profiling"]["canDggbHit()"];
                    d = d.asDouble() + double(clock() - startTime) / CLOCKS_PER_SEC;
#endif
                        return false;
                    }
                    if (r == riR)
                    {
                        hitDir = shootDown;
#ifdef PROFILING
                        auto &&d = Debug::debugData["profiling"]["canDggbHit()"];
                    d = d.asDouble() + double(clock() - startTime) / CLOCKS_PER_SEC;
#endif
                        return true;
                    }
                }
                r = myR;
                while (!(fieldStatic[r][myC] & direction2OpposingWall[up]))
                {
                    r = (r - 1 + height) % height;
                    if (r == riR)
                    {
                        hitDir = shootUp;
#ifdef PROFILING
                        auto &&d = Debug::debugData["profiling"]["canDggbHit()"];
                    d = d.asDouble() + double(clock() - startTime) / CLOCKS_PER_SEC;
#endif
                        return true;
                    }
                }
#ifdef PROFILING
                auto &&d = Debug::debugData["profiling"]["canDggbHit()"];
            d = d.asDouble() + double(clock() - startTime) / CLOCKS_PER_SEC;
#endif
                return false;
            } else if (myR == riR)
            {
                int c = myC;
                while (!(fieldStatic[myR][c] & direction2OpposingWall[right]))
                {
                    c = (c + 1 + width) % width;
                    if (c == myC)
                    {
#ifdef PROFILING
                        auto &&d = Debug::debugData["profiling"]["canDggbHit()"];
                    d = d.asDouble() + double(clock() - startTime) / CLOCKS_PER_SEC;
#endif
                        return false;
                    }
                    if (c == riC)
                    {
                        hitDir = shootRight;
#ifdef PROFILING
                        auto &&d = Debug::debugData["profiling"]["canDggbHit()"];
                    d = d.asDouble() + double(clock() - startTime) / CLOCKS_PER_SEC;
#endif
                        return true;
                    }
                }
                c = myC;
                while (!(fieldStatic[myR][c] & direction2OpposingWall[left]))
                {
                    c = (c - 1 + width) % width;
                    if (c == riC)
                    {
                        hitDir = shootLeft;
#ifdef PROFILING
                        auto &&d = Debug::debugData["profiling"]["canDggbHit()"];
                    d = d.asDouble() + double(clock() - startTime) / CLOCKS_PER_SEC;
#endif
                        return true;
                    }
                }
#ifdef PROFILING
                auto &&d = Debug::debugData["profiling"]["canDggbHit()"];
            d = d.asDouble() + double(clock() - startTime) / CLOCKS_PER_SEC;
#endif
                return false;
            }
#ifdef PROFILING
            auto &&d = Debug::debugData["profiling"]["canDggbHit()"];
        d = d.asDouble() + double(clock() - startTime) / CLOCKS_PER_SEC;
#endif
            return false;
        }

        bool canDggbHit(Direction dir, int myID, int rivalID)
        {
#ifdef PROFILING
            auto &&startTime = clock();
#endif
            const auto& me = players[myID], rival = players[rivalID];
            int myR, myC;
            if (dir == stay)
                myR = me.row, myC = me.col;
            else
                myR = (me.row + dy[dir] + height) % height, myC = (me.col + dx[dir] + width) % width;
            const int riR = rival.row, riC = rival.col;
            if (myC == riC)
            {
                if (myR == riR)
                    return false;
                int r = myR;
                while (!(fieldStatic[r][myC] & direction2OpposingWall[down]))
                {
                    r = (r + 1 + height) % height;
                    if (r == myR)
                    {
#ifdef PROFILING
                        auto &&d = Debug::debugData["profiling"]["canDggbHit()"];
                    d = d.asDouble() + double(clock() - startTime) / CLOCKS_PER_SEC;
#endif
                        return false;
                    }
                    if (r == riR)
                    {
                        hitDir = shootDown;
#ifdef PROFILING
                        auto &&d = Debug::debugData["profiling"]["canDggbHit()"];
                    d = d.asDouble() + double(clock() - startTime) / CLOCKS_PER_SEC;
#endif
                        return true;
                    }
                }
                r = myR;
                while (!(fieldStatic[r][myC] & direction2OpposingWall[up]))
                {
                    r = (r - 1 + height) % height;
                    if (r == riR)
                    {
                        hitDir = shootUp;
#ifdef PROFILING
                        auto &&d = Debug::debugData["profiling"]["canDggbHit()"];
                    d = d.asDouble() + double(clock() - startTime) / CLOCKS_PER_SEC;
#endif
                        return true;
                    }
                }
#ifdef PROFILING
                auto &&d = Debug::debugData["profiling"]["canDggbHit()"];
            d = d.asDouble() + double(clock() - startTime) / CLOCKS_PER_SEC;
#endif
                return false;
            } else if (myR == riR)
            {
                int c = myC;
                while (!(fieldStatic[myR][c] & direction2OpposingWall[right]))
                {
                    c = (c + 1 + width) % width;
                    if (c == myC)
                    {
#ifdef PROFILING
                        auto &&d = Debug::debugData["profiling"]["canDggbHit()"];
                    d = d.asDouble() + double(clock() - startTime) / CLOCKS_PER_SEC;
#endif
                        return false;
                    }
                    if (c == riC)
                    {
                        hitDir = shootRight;
#ifdef PROFILING
                        auto &&d = Debug::debugData["profiling"]["canDggbHit()"];
                    d = d.asDouble() + double(clock() - startTime) / CLOCKS_PER_SEC;
#endif
                        return true;
                    }
                }
                c = myC;
                while (!(fieldStatic[myR][c] & direction2OpposingWall[left]))
                {
                    c = (c - 1 + width) % width;
                    if (c == riC)
                    {
                        hitDir = shootLeft;
#ifdef PROFILING
                        auto &&d = Debug::debugData["profiling"]["canDggbHit()"];
                    d = d.asDouble() + double(clock() - startTime) / CLOCKS_PER_SEC;
#endif
                        return true;
                    }
                }
#ifdef PROFILING
                auto &&d = Debug::debugData["profiling"]["canDggbHit()"];
            d = d.asDouble() + double(clock() - startTime) / CLOCKS_PER_SEC;
#endif
                return false;
            }
#ifdef PROFILING
            auto &&d = Debug::debugData["profiling"]["canDggbHit()"];
        d = d.asDouble() + double(clock() - startTime) / CLOCKS_PER_SEC;
#endif
            return false;
        }

        bool canDggbHitR(Direction dir, int myID, int rivalID)
        {
#ifdef PROFILING
            auto &&startTime = clock();
#endif
            const auto& me = players[myID], rival = players[rivalID];
            int riR, riC;
            if (dir == stay)
                riR = rival.row, riC = rival.col;
            else
                riR = (rival.row + dy[dir] + height) % height, riC = (rival.col + dx[dir] + width) % width;
            const int myR = me.row, myC = me.col;
            if (myC == riC)
            {
                if (myR == riR)
                    return false;
                int r = myR;
                while (!(fieldStatic[r][myC] & direction2OpposingWall[down]))
                {
                    r = (r + 1 + height) % height;
                    if (r == myR)
                    {
#ifdef PROFILING
                        auto &&d = Debug::debugData["profiling"]["canDggbHit()"];
                    d = d.asDouble() + double(clock() - startTime) / CLOCKS_PER_SEC;
#endif
                        return false;
                    }
                    if (r == riR)
                    {
                        hitDir = shootDown;
#ifdef PROFILING
                        auto &&d = Debug::debugData["profiling"]["canDggbHit()"];
                    d = d.asDouble() + double(clock() - startTime) / CLOCKS_PER_SEC;
#endif
                        return true;
                    }
                }
                r = myR;
                while (!(fieldStatic[r][myC] & direction2OpposingWall[up]))
                {
                    r = (r - 1 + height) % height;
                    if (r == riR)
                    {
                        hitDir = shootUp;
#ifdef PROFILING
                        auto &&d = Debug::debugData["profiling"]["canDggbHit()"];
                    d = d.asDouble() + double(clock() - startTime) / CLOCKS_PER_SEC;
#endif
                        return true;
                    }
                }
#ifdef PROFILING
                auto &&d = Debug::debugData["profiling"]["canDggbHit()"];
            d = d.asDouble() + double(clock() - startTime) / CLOCKS_PER_SEC;
#endif
                return false;
            } else if (myR == riR)
            {
                int c = myC;
                while (!(fieldStatic[myR][c] & direction2OpposingWall[right]))
                {
                    c = (c + 1 + width) % width;
                    if (c == myC)
                    {
#ifdef PROFILING
                        auto &&d = Debug::debugData["profiling"]["canDggbHit()"];
                    d = d.asDouble() + double(clock() - startTime) / CLOCKS_PER_SEC;
#endif
                        return false;
                    }
                    if (c == riC)
                    {
                        hitDir = shootRight;
#ifdef PROFILING
                        auto &&d = Debug::debugData["profiling"]["canDggbHit()"];
                    d = d.asDouble() + double(clock() - startTime) / CLOCKS_PER_SEC;
#endif
                        return true;
                    }
                }
                c = myC;
                while (!(fieldStatic[myR][c] & direction2OpposingWall[left]))
                {
                    c = (c - 1 + width) % width;
                    if (c == riC)
                    {
                        hitDir = shootLeft;
#ifdef PROFILING
                        auto &&d = Debug::debugData["profiling"]["canDggbHit()"];
                    d = d.asDouble() + double(clock() - startTime) / CLOCKS_PER_SEC;
#endif
                        return true;
                    }
                }
#ifdef PROFILING
                auto &&d = Debug::debugData["profiling"]["canDggbHit()"];
            d = d.asDouble() + double(clock() - startTime) / CLOCKS_PER_SEC;
#endif
                return false;
            }
#ifdef PROFILING
            auto &&d = Debug::debugData["profiling"]["canDggbHit()"];
        d = d.asDouble() + double(clock() - startTime) / CLOCKS_PER_SEC;
#endif
            return false;
        }

        //weaZen: 地图分析
        //Jet: 分析HOTSPOT
        void MapAnalyze()
        {
#ifdef PROFILING
            auto &&startTime = clock();
#endif
            //分析PathInfo
            FieldProp deadSpot[40];
            int degree[FIELD_MAX_HEIGHT][FIELD_MAX_HEIGHT];
            auto dCount = 0;
            PathInfoType *ptmpExit = nullptr;

            for (int y = 0; y < height; ++y)
            {
                for (int x = 0; x < width; ++x)
                {
                    pathInfo[y][x].row = y;
                    pathInfo[y][x].col = x;
                    auto degreeCount = 0;
                    for (auto dir = up; dir < 4; ++dir)
                        if (!(fieldStatic[y][x] & direction2OpposingWall[dir]))
                            ++degreeCount;
                    degree[y][x] = degreeCount;
                    if (degreeCount == 1)
                    {
                        pathInfo[y][x].isImpasse = true;
                        deadSpot[dCount].row = y;
                        deadSpot[dCount++].col = x;
                    }
                }
            }
            for (int i = 0; i < dCount; ++i)
            {
                auto startPos = deadSpot[i];
                FieldProp queue[QUEUE_MAX];
                queue[0] = startPos;
                auto nowFlag = 0, endFlag = 0;
                while (nowFlag <= endFlag)
                {
                    const auto& curGrid = fieldStatic[queue[nowFlag].row][queue[nowFlag].col];
                    for (auto dir = up; dir < 4; ++dir)
                        if (!(curGrid & direction2OpposingWall[dir]))
                        {
                            auto newPos = queue[nowFlag];
                            newPos.row = (newPos.row + dy[dir] + height) % height;
                            newPos.col = (newPos.col + dx[dir] + width) % width;
                            --degree[newPos.row][newPos.col];
                            if (degree[newPos.row][newPos.col] == 1 && !pathInfo[newPos.row][newPos.col].isImpasse)
                            {
                                pathInfo[newPos.row][newPos.col].isExit = false;
                                pathInfo[newPos.row][newPos.col].isImpasse = true;
                                queue[++endFlag] = newPos;
                            }
                            if (degree[newPos.row][newPos.col] > 1)
                            {
                                pathInfo[newPos.row][newPos.col].isExit = true;
                                pathInfo[newPos.row][newPos.col].pExit = ptmpExit = &pathInfo[newPos.row][newPos.col];
                                pathInfo[newPos.row][newPos.col].fleeLength = 0;
                            }
                        }
                    ++nowFlag;
                }
                for (auto j = 0; j <= endFlag; ++j)
                {
                    pathInfo[queue[j].row][queue[j].col].fleeLength = endFlag - j + 1;
                    pathInfo[queue[j].row][queue[j].col].pExit = ptmpExit;
                }
                (*ptmpExit).impasseDepth = endFlag + 1;
            }
            for (auto y = 0; y < height; ++y)
            {
                for (auto x = 0; x < width; ++x)
                {
                    if (pathInfo[y][x].isImpasse)
                    {
                        ptmpExit = pathInfo[y][x].pExit;
                        while (ptmpExit != ptmpExit->pExit)
                        {
                            ptmpExit->pExit->impasseDepth = std::max(ptmpExit->pExit->impasseDepth,
                                                                     ptmpExit->impasseDepth + ptmpExit->fleeLength);
                            ptmpExit->impasseDepth = 0;
                            pathInfo[y][x].pExit = ptmpExit->pExit;
                            pathInfo[y][x].fleeLength += ptmpExit->fleeLength;
                            ptmpExit = pathInfo[y][x].pExit;
                        }
                    }
                }
            }

            //weaZen正在尝试精确分析hotspot
            maxCluster = 0;
            int fruitSpotsCount = 0;
            FieldProp fruitSpots[4 * 8];
            for (int i = 0; i < generatorCount; i++)
                for (auto d = up; d < 8; ++d)
                {
                    int tmpy = (generators[i].row + dy[d] + height) % height;
                    int tmpx = (generators[i].col + dx[d] + width) % width;
                    if ((fieldStatic[tmpy][tmpx] & generator) || genInfo[tmpy][tmpx].isBesideGen)
                        continue;
                    genInfo[tmpy][tmpx].isBesideGen = true;
                    fruitSpots[fruitSpotsCount].row = tmpy;
                    fruitSpots[fruitSpotsCount++].col = tmpx;
                }
            for (int i = 0; i < fruitSpotsCount; ++i)
            {
                if (genInfo[fruitSpots[i].row][fruitSpots[i].col].fruitClusterCount > 0)
                    continue;
                FieldProp cluster[32];
                int clusterCount = 1;
                cluster[0] = fruitSpots[i];
                genInfo[fruitSpots[i].row][fruitSpots[i].col].fruitClusterCount = 1;
                int nowFlag = 0, endFlag = 0;
                while (nowFlag <= endFlag)
                {
                    for (int checkSpot = i + 1; checkSpot < fruitSpotsCount; ++checkSpot)
                    {
                        if (genInfo[fruitSpots[checkSpot].row][fruitSpots[checkSpot].col].fruitClusterCount == 0 &&
                            Distance(cluster[nowFlag], fruitSpots[checkSpot]) <= 3)
                        {
                            genInfo[fruitSpots[checkSpot].row][fruitSpots[checkSpot].col].fruitClusterCount = 1;
                            cluster[++endFlag] = fruitSpots[checkSpot];
                            ++clusterCount;
                        }
                    }
                    ++nowFlag;
                }
                for (int j = 0; j < clusterCount; ++j)
                    genInfo[cluster[j].row][cluster[j].col].fruitClusterCount = clusterCount;
                maxCluster = std::max(maxCluster, clusterCount);
            }
#ifdef DEBUG
            for (int i = 0; i < height; i++)
        {
            for (int j = 0; j < width; j++)
            {
                cout << genInfo[i][j].fruitClusterCount << '\t';
            }
            cout << endl;
        }
#endif // DEBUG

#ifdef PROFILING
            auto &&d = Debug::debugData["profiling"]["MapAnalyze()"];
        d = d.asDouble() + double(clock() - startTime) / CLOCKS_PER_SEC;
#endif
        }

        // 读取并解析程序输入，本地调试或提交平台使用都可以。
        // 如果在本地调试，程序会先试着读取参数中指定的文件作为输入文件，失败后再选择等待用户直接输入。
        // 本地调试时可以接受多行以便操作，Windows下可以用 Ctrl-Z 或一个【空行+回车】表示输入结束，但是在线评测只需接受单行即可。
        // localFileName 可以为NULL
        // obtainedData 会输出自己上回合存储供本回合使用的数据
        // obtainedGlobalData 会输出自己的 Bot 上以前存储的数据
        // 返回值是自己的 playerID
        int ReadInput(const char *localFileName, Json::Value& obtainedData, Json::Value& obtainedGlobalData)
        {
            string str, chunk;
            if (!Debug::presetString.empty())
                str = Debug::presetString;
            else
            {
#ifdef _BOTZONE_ONLINE
                std::ios::sync_with_stdio(false); //ω\\)
            getline(cin, str);
#else
                if (localFileName)
                {
                    std::ifstream fin(localFileName);
                    if (fin)
                        while (getline(fin, chunk) && chunk != "")
                            str += chunk;
                    else
                        while (getline(cin, chunk) && chunk != "")
                            str += chunk;
                } else
                    while (getline(cin, chunk) && chunk != "")
                        str += chunk;
#endif
            }

            Json::Reader reader;
            Json::Value input;
            reader.parse(str, input);

            int len = input["requests"].size();

            // 读取场地静态状况
            auto field = input["requests"][Json::Value::UInt(0)],
                    staticField = field["static"],  // 墙面和产生器
                    contentField = field["content"]; // 豆子和玩家
            height = field["height"].asInt();
            width = field["width"].asInt();
            LARGE_FRUIT_DURATION = field["LARGE_FRUIT_DURATION"].asInt();
            LARGE_FRUIT_ENHANCEMENT = field["LARGE_FRUIT_ENHANCEMENT"].asInt();
            SKILL_COST = field["SKILL_COST"].asInt();
            generatorTurnLeft = GENERATOR_INTERVAL = field["GENERATOR_INTERVAL"].asInt();

            PrepareInitialField(staticField, contentField);

            hasNext = true;
            largeFruitValue = width + height <= 15 ? 15 : 10;

            MapAnalyze();

            // 根据历史恢复局面
            for (int i = 1; i < len; i++)
            {
                for (int _ = 0; _ < MAX_PLAYER_COUNT; _++)
                    if (!players[_].dead)
                        actions[_] = Direction(input["requests"][i][playerID2str[_]]["action"].asInt());
                NextTurn();
            }

            obtainedData = input["data"];
            obtainedGlobalData = input["globaldata"];

            return field["id"].asInt();
        }

        // 根据 static 和 content 数组准备场地的初始状况
        void PrepareInitialField(const Json::Value& staticField, const Json::Value& contentField)
        {
            int r, c, gid = 0;
            generatorCount = 0;
            aliveCount = 0;
            smallFruitCount = 0;
            generatorTurnLeft = GENERATOR_INTERVAL;
            for (r = 0; r < height; r++)
                for (c = 0; c < width; c++)
                {
                    auto& content = fieldContent[r][c] = GridContentType(contentField[r][c].asInt());
                    auto& s = fieldStatic[r][c] = GridStaticType(staticField[r][c].asInt());
                    if (s & generator)
                    {
                        generators[gid].row = r;
                        generators[gid++].col = c;
                        generatorCount++;
                    }
                    if (content & smallFruit)
                        smallFruitCount++;
                    for (int _ = 0; _ < MAX_PLAYER_COUNT; _++)
                        if (content & playerID2Mask[_])
                        {
                            auto& p = players[_];
                            p.col = c;
                            p.row = r;
                            p.powerUpLeft = 0;
                            p.strength = 1;
                            p.dead = false;
                            aliveCount++;
                        }
                }
        }

        // 完成决策，输出结果。
        // action 表示本回合的移动方向，stay 为不移动
        // tauntText 表示想要叫嚣的言语，可以是任意字符串，除了显示在屏幕上不会有任何作用，留空表示不叫嚣
        // data 表示自己想存储供下一回合使用的数据，留空表示删除
        // globalData 表示自己想存储供以后使用的数据（替换），这个数据可以跨对局使用，会一直绑定在这个 Bot 上，留空表示删除
        // Jet: debugData为一个Json对象，botzone上不打印，用于本地调试
        void WriteOutput(Direction action, string& tauntText,
                         Json::Value& data, Json::Value& globalData, Json::Value& debugData) const
        {
#ifdef PROFILING
            auto &&startTime = clock();
#endif

            debugData["seed"] = to_string(seed);

#ifdef PROFILING
            auto &&d = Debug::debugData["profiling"]["WriteOutput()"];
        d = d.asDouble() + double(clock() - startTime) / CLOCKS_PER_SEC;
#endif

            Json::Value ret;
            ret["response"]["action"] = action;
            ret["response"]["tauntText"] = tauntText;
            ret["data"] = data;
            ret["globaldata"] = globalData;
            if (Debug::printInfo)
                ret["debug"] = debugData;

#ifdef _BOTZONE_ONLINE
            Json::FastWriter writer; // 在线评测的话能用就行……
#else
            Json::StyledWriter writer; // 本地调试这样好看 > <
#endif
            cout << writer.write(ret) << endl;
        }

        // 用于显示当前游戏状态，调试用。
        // 提交到平台后会被优化掉。
        inline void DebugPrint() const
        {
#ifndef _BOTZONE_ONLINE
            printf("回合号【%d】存活人数【%d】| 图例 产生器[G] 有玩家[0/1/2/3] 多个玩家[*] 大豆[o] 小豆[.]\n", turnID, aliveCount);
            for (int _ = 0; _ < MAX_PLAYER_COUNT; _++)
            {
                auto& p = players[_];
                printf("[玩家%d(%d, %d)|力量%d|加成剩余回合%d|%s]\n",
                       _, p.row, p.col, p.strength, p.powerUpLeft, p.dead ? "死亡" : "存活");
            }
            putchar(' ');
            putchar(' ');
            for (int c = 0; c < width; c++)
                printf("  %d ", c);
            putchar('\n');
            for (int r = 0; r < height; r++)
            {
                putchar(' ');
                putchar(' ');
                for (int c = 0; c < width; c++)
                {
                    putchar(' ');
                    printf((fieldStatic[r][c] & wallNorth) ? "---" : "   ");
                }
                printf("\n%d ", r);
                for (int c = 0; c < width; c++)
                {
                    putchar((fieldStatic[r][c] & wallWest) ? '|' : ' ');
                    putchar(' ');
                    int hasPlayer = -1;
                    for (int _ = 0; _ < MAX_PLAYER_COUNT; _++)
                        if (fieldContent[r][c] & playerID2Mask[_])
                            if (hasPlayer == -1)
                                hasPlayer = _;
                            else
                                hasPlayer = 4;
                    if (hasPlayer == 4)
                        putchar('*');
                    else if (hasPlayer != -1)
                        putchar('0' + hasPlayer);
                    else if (fieldStatic[r][c] & generator)
                        putchar('G');
                    else if (fieldContent[r][c] & playerMask)
                        putchar('*');
                    else if (fieldContent[r][c] & smallFruit)
                        putchar('.');
                    else if (fieldContent[r][c] & largeFruit)
                        putchar('o');
                    else
                        putchar(' ');
                    putchar(' ');
                }
                putchar((fieldStatic[r][width - 1] & wallEast) ? '|' : ' ');
                putchar('\n');
            }
            putchar(' ');
            putchar(' ');
            for (int c = 0; c < width; c++)
            {
                putchar(' ');
                printf((fieldStatic[height - 1][c] & wallSouth) ? "---" : "   ");
            }
            putchar('\n');
#endif
        }

        Json::Value SerializeCurrentTurnChange()
        {
            Json::Value result;
            TurnStateTransfer& bt = backtrack[turnID - 1];
            for (int _ = 0; _ < MAX_PLAYER_COUNT; _++)
            {
                result["actions"][_] = bt.actions[_];
                result["strengthDelta"][_] = bt.strengthDelta[_];
                result["change"][_] = bt.change[_];
            }
            return result;
        }

        // 初始化游戏管理器
        GameField()
        {
            if (constructed)
                throw runtime_error("请不要再创建 GameField 对象了，整个程序中只应该有一个对象");
            constructed = true;

            turnID = 0;
        }

        //GameField(const GameField &b) : GameField() { }
    };

    bool GameField::constructed = false;
}

// 一些辅助程序
namespace Helpers
{
    using namespace EnumExt;

    int MY_ID = -1;

    class Solution : public std::pair<Pacman::Direction, int>
    {
    public:
        Solution() { second = -100000000; }

        Solution(const std::pair<Pacman::Direction, int>& p) : std::pair<Pacman::Direction, int>(p) {}

        bool operator<(const Solution& o) const { return second < o.second; }
    };

    inline int RandBetween(int a, int b)
    {
        if (a > b)
            swap(a, b);
        return rand() % (b - a) + a;
    }

    inline int DeltaATK(const Pacman::GameField& gamefield, int id1, int id2)
    {
        return gamefield.players[id1].strength - gamefield.players[id2].strength;
    }

// weaZen:简单的危险判断
    bool SimpleDangerJudge(Pacman::GameField& gameField, int myID, Pacman::Direction myDir = Pacman::stay)
    {
        const Pacman::Player& me = gameField.players[myID];
        Pacman::FieldProp myPos;
        if (myDir > Pacman::Direction::stay && myDir < Pacman::Direction::shootUp)
        {
            myPos.row = (me.row + Pacman::dy[myDir] + gameField.height) % gameField.height;
            myPos.col = (me.col + Pacman::dx[myDir] + gameField.width) % gameField.width;
        } else
            myPos = me;
        for (int _ = 0; _ < 4; ++_)
        {
            int d = DeltaATK(gameField, myID, _);
            if (d < 0 && gameField.Distance(myPos, gameField.players[_]) <= 1)
                return true;
        }
        return false;
    }

//weaZen:随便找个不被吃的方向(如果可以)
//Jet:需要返回随机值的情况大多可以返回AI::RandomAI()代替
    Pacman::Direction SimpleRandom(Pacman::GameField& gameField, int myID, char forbiddenDirs = '\0')
    {
        Pacman::Direction dir;
        int vCount = 0;
        Pacman::Direction valid[5];
        for (Pacman::Direction d = Pacman::stay; d <= Pacman::left; ++d)
            if (gameField.ActionValid(myID, d) && !(forbiddenDirs & (1 << (d + 1))))
                valid[vCount++] = d;
        if (vCount == 0)
            return Pacman::Direction::stay;
        dir = valid[RandBetween(0, vCount)];
        return dir;
    }

//weaZen:递归统计死路上某一位置之后 一条路径上的最大可能水果数量 给Naive快速判断是否尝试吃人用
    int fruitCount(Pacman::GameField& gameField, Pacman::FieldProp pos)
    {
        if (!gameField.pathInfo[pos.row][pos.col].isImpasse && !gameField.pathInfo[pos.row][pos.col].isExit)
            return 0;
        int max = 0;
        Pacman::FieldProp nextPos;
        for (Pacman::Direction dir = Pacman::up; dir <= Pacman::left; ++dir)
        {
            if (gameField.fieldStatic[pos.row][pos.col] & Pacman::direction2OpposingWall[dir])
                continue;
            nextPos.row = (pos.row + Pacman::dy[dir] + gameField.height) % gameField.height;
            nextPos.col = (pos.col + Pacman::dx[dir] + gameField.width) % gameField.width;
            if (gameField.pathInfo[nextPos.row][nextPos.col].fleeLength ==
                gameField.pathInfo[pos.row][pos.col].fleeLength + 1)
                max = std::max(max, fruitCount(gameField, nextPos));
        }
        if (gameField.fieldContent[pos.row][pos.col] & Pacman::smallFruit)
            ++max;
        if (gameField.fieldContent[pos.row][pos.col] & Pacman::largeFruit)
            max += 10;
        return max;
    }

    inline string depth2String(int depth)
    {
        auto&& str = to_string(depth);
        if (str.length() == 1)
            str.insert(str.begin(), '0');
        return "depth = " + str;
    }
}

namespace AI
{
    using namespace EnumExt;
    using Helpers::Solution;
    static int maxDepth;

//DangerInfoType: 表示若干回合后会死亡，不合法行为记为0回合(当场死亡)。
    typedef std::pair<Pacman::Direction, int> DangerInfoType;

    std::vector<Solution> tmpSol(5);
    std::vector<DangerInfoType> tmpDangers(5);

    int SimpleSearch(Pacman::GameField& gameField, int myID, int depth,
                     Pacman::Direction (*rivalAI)(Pacman::GameField&, int, int),
                     Pacman::Direction lastDir = Pacman::stay, std::vector<Solution>& solutions = tmpSol);

//Jet: 可以射的更优先（他们很野蛮）
//weaZen： 目标优先级：在死路上可能逃不出来的弱AI > 附近在死路出口的弱AI、附近在夹道中被追击的弱AI > 大果子 > 小果子 > 生成器 判断危险用的对手AI
    Pacman::Direction NaiveAttackAI(Pacman::GameField& gameField, int myID, int targetID)
    {
        Pacman::Player& me = gameField.players[myID];

        char largeFruitTarget = Pacman::GridContentType::largeFruit;
        char smallFruitTarget = Pacman::GridContentType::smallFruit;
        char playerTarget = 0;
        char tryPlayerTarget = 0;
        char forbiddenDirs = '\0';
        Pacman::Direction dir;

        for (int i = -1; i < 4; ++i)
        {
            if (!gameField.ActionValid(myID, Pacman::Direction(i)))
            {
                forbiddenDirs |= 1 << (i + 1);
                continue;
            }
            if (Helpers::SimpleDangerJudge(gameField, myID, Pacman::Direction(i)))
                forbiddenDirs |= 1 << (i + 1);
        }
        if (forbiddenDirs == 31) //（基本）必死无疑
            return Helpers::SimpleRandom(gameField, myID);

//         if (me.strength >= gameField.SKILL_COST)
//         {
//             if (gameField.canDggbHit(myID, Helpers::MY_ID) && gameField.Distance(myID, Helpers::MY_ID) >= 2)
//                 return gameField.hitDir;
//             for (int _ = 0; _ < MAX_PLAYER_COUNT; _++)
//             {
//                 if (_ == myID || _ == Helpers::MY_ID)
//                     continue;
//                 if (gameField.canDggbHit(myID, _) && gameField.Distance(myID, _) >= 2)
//                     return gameField.hitDir;
//             }
//         }

        for (int _ = 0; _ < MAX_PLAYER_COUNT; _++)
        {
            Pacman::Player& rival = gameField.players[_];
            if (_ == myID || rival.dead)
                continue;
            if (Helpers::DeltaATK(gameField, myID, _) > 0)
            {
                bool preyFlag = gameField.pathInfo[rival.row][rival.col].isImpasse &&
                                gameField.pathInfo[rival.row][rival.col].fleeLength + 2 >=
                                gameField.Distance(me, *gameField.pathInfo[rival.row][rival.col].pExit);
                bool tryPreyFlag = gameField.pathInfo[rival.row][rival.col].isExit && gameField.Distance(myID, _) <= 2;
                //夹道里被追击的弱AI
                if (!preyFlag && !tryPreyFlag)
                {
                    int dirCount = 4;
                    for (int i = 0; i < 4; ++i)
                    {
                        if (gameField.fieldStatic[rival.row][rival.col] & Pacman::direction2OpposingWall[i])
                        {
                            --dirCount;
                            continue;
                        }
                        Pacman::FieldProp checkPos;
                        checkPos.row = (rival.row + Pacman::dy[i] + gameField.height) % gameField.height;
                        checkPos.col = (rival.col + Pacman::dx[i] + gameField.width) % gameField.width;
                        if (gameField.fieldContent[checkPos.row][checkPos.col] & Pacman::playerMask)
                            for (int checkID = 0; checkID < 4; ++checkID)
                            {
                                if (checkID == myID)
                                    continue;
                                if (gameField.fieldContent[checkPos.row][checkPos.col] &
                                    Pacman::playerID2Mask[checkID] &&
                                    gameField.players[checkID].strength > rival.strength)
                                {
                                    --dirCount;
                                    break;
                                }
                            }
                    }
                    if (dirCount == 1 && gameField.Distance(myID, _) <= 2 && (me.strength - rival.strength > 4))
                        tryPreyFlag = true;
                }
                if (preyFlag)
                    playerTarget |= Pacman::playerID2Mask[_];
                if (tryPreyFlag)
                    tryPlayerTarget |= Pacman::playerID2Mask[_];
            }
        }

        if (targetID != -1)
        {
            if (playerTarget & Pacman::playerID2Mask[targetID])
                playerTarget = Pacman::playerID2Mask[targetID];
            if (tryPlayerTarget & Pacman::playerID2Mask[targetID])
                tryPlayerTarget = Pacman::playerID2Mask[targetID];
        }

        auto&& smallFruitInfo = gameField.GetToTarget(myID, smallFruitTarget, forbiddenDirs);
        auto&& largeFruitInfo = gameField.GetToTarget(myID, largeFruitTarget, forbiddenDirs);
        auto&& playerInfo = gameField.GetToTarget(myID, playerTarget, forbiddenDirs);
        auto&& tryPlayerInfo = gameField.GetToTarget(myID, tryPlayerTarget, forbiddenDirs);
        //一定概率放弃当前果子
        if (smallFruitInfo.dis == 0 && Helpers::RandBetween(0, 2))
            smallFruitInfo = gameField.GetToTarget(myID, smallFruitTarget, forbiddenDirs | 1);
        if (largeFruitInfo.dis == 0 && Helpers::RandBetween(0, 2))
            largeFruitInfo = gameField.GetToTarget(myID, largeFruitTarget, forbiddenDirs | 1);
#ifdef DEBUG
        //cout << '#' << myID << ' ' << (fruitInfo >> 3) << ' ' << Pacman::dirStr[fruitInfo & 7] << ' ' << (playerInfo >> 3) << ' ' << Pacman::dirStr[playerInfo & 7] << endl;
#endif // DEBUG
        auto smallFruitDirInfo = smallFruitInfo.dir;
        auto largeFruitDirInfo = largeFruitInfo.dir;
        auto playerDirInfo = playerInfo.dir;
        auto tryPlayerDirInfo = tryPlayerInfo.dir;

        int info = (smallFruitDirInfo < 5) + ((largeFruitDirInfo < 5) << 1) + ((tryPlayerDirInfo < 5) << 2) +
                   ((playerDirInfo < 5) << 3);

        if (info >= 8)
            dir = playerDirInfo;
        else if (info >= 4)
            dir = tryPlayerDirInfo;
        else if (info >= 2)
            //他们对大果子有狂热的爱
            dir = largeFruitDirInfo;
        else if (info >= 1 && smallFruitInfo.dis <= gameField.generatorTurnLeft)
            dir = smallFruitDirInfo;
        else
            dir = gameField.GetToNearbyGenerator(myID, forbiddenDirs).dir;

        if (dir != Pacman::stay && dir != Pacman::Direction::noPos)
            return dir;
        //为了能够搜索减少耗时直接随机
        if (Helpers::RandBetween(0, 4) <= 2 && dir == Pacman::Direction::stay)
            return dir;
        else
            return Helpers::SimpleRandom(gameField, myID, forbiddenDirs);
    }

//weaZen: 复杂的危险判断
    void DangerJudge(Pacman::GameField& gameField, int myID, std::vector<DangerInfoType>& dangers, int maxDepth = 10)
    {
        dangers.clear();
        for (Pacman::Direction dir = Pacman::stay; dir <= Pacman::left; ++dir)
        {
            if (!gameField.ActionValid(myID, dir))
            {
                dangers.push_back(std::make_pair(dir, 0));
                continue;
            }
            if (Helpers::SimpleDangerJudge(gameField, myID, dir))
            {
                dangers.push_back(std::make_pair(dir, 1));
                continue;
            }
            Pacman::FieldProp nextGrid;
            if (dir != Pacman::Direction::stay)
            {
                nextGrid.row = (gameField.players[myID].row + Pacman::dy[dir] + gameField.height) % gameField.height;
                nextGrid.col = (gameField.players[myID].col + Pacman::dx[dir] + gameField.width) % gameField.width;
            } else
                nextGrid = gameField.players[myID];

            if (gameField.pathInfo[nextGrid.row][nextGrid.col].isImpasse)
            {
                int fleeLength = gameField.pathInfo[nextGrid.row][nextGrid.col].fleeLength;
                bool enemyFlag = false;

                //死路出口附近没有其他人时不用搜索了
                for (int _ = 0; _ < MAX_PLAYER_COUNT; _++)
                {
                    if (_ == myID)
                        continue;
                    if (gameField.Distance(gameField.players[_],
                                           *gameField.pathInfo[nextGrid.row][nextGrid.col].pExit) <= fleeLength + 1)
                        enemyFlag = true;
                }
                if (!enemyFlag)
                    continue;

                //注意只有一个gamefield 模拟其他AI时注意action的还原
                Pacman::Direction tmpDir[MAX_PLAYER_COUNT];
                for (int _ = 0; _ < MAX_PLAYER_COUNT; _++)
                    tmpDir[_] = gameField.actions[_];
                for (int _ = 0; _ < MAX_PLAYER_COUNT; _++)
                {
                    if (_ == myID)
                        continue;
                    if (gameField.players[_].dead)
                        continue;
                    gameField.actions[_] = NaiveAttackAI(gameField, _, myID);
                }
                gameField.actions[myID] = dir;
                gameField.NextTurn();
                for (int i = 0; i < 5; i++)
                {
                    tmpSol[i].first = Pacman::Direction(i - 1);
                    tmpSol[i].second = 0;
                }

                int searchDepth = std::min(maxDepth,
                                           gameField.pathInfo[nextGrid.row][nextGrid.col].pExit->impasseDepth +
                                           fleeLength + 1);

                for (int tmpDepth = 1; tmpDepth <= searchDepth; ++tmpDepth)
                {
                    if (SimpleSearch(gameField, myID, tmpDepth, NaiveAttackAI) <= DEATH_EVAL)
                    {
                        dangers.push_back(std::make_pair(dir, tmpDepth + 1));
                        break;
                    }
                }
                gameField.RollBack(1);
                for (int _ = 0; _ < MAX_PLAYER_COUNT; _++)
                    gameField.actions[_] = tmpDir[_];
            }
        }
    }

//weaZen： 会回避死亡的高级AI
    Pacman::Direction NaiveThinkAI(Pacman::GameField& gameField, int myID, int targetID)
    {
        char fruitTarget = (Pacman::GridContentType::smallFruit | Pacman::GridContentType::largeFruit);
        char playerTarget = 0;
        char tryPlayerTarget = 0;
        char forbiddenDirs = '\0';
        Pacman::Direction dir;

        DangerJudge(gameField, myID, tmpDangers, 5);

        if (tmpDangers.size() == 5) //（基本）必死无疑
        {
            //随机选最晚死的方向
            int tmpMax = 0;
            int maxCount = 1;
            for (auto i = 0; i < 5; ++i)
            {
                if (tmpDangers[i].second > tmpMax)
                {
                    tmpMax = tmpDangers[i].second;
                    dir = tmpDangers[i].first;
                    maxCount = 1;
                } else if (tmpDangers[i].second == tmpMax)
                {
                    dir = rand() % ++maxCount ? dir : tmpDangers[i].first;
                }
            }

            //如果在死路上且敌人在死路外尝试逃离
            if (gameField.pathInfo[gameField.players[myID].row][gameField.players[myID].col].isImpasse)
            {
                //到出口的方向是fleeLength减1的位置
                Pacman::Direction fleeDir;
                for (fleeDir = Pacman::up; fleeDir <= Pacman::left; ++fleeDir)
                {
                    int tmpx, tmpy;
                    tmpy = (gameField.players[myID].row + Pacman::dy[fleeDir] + gameField.height) % gameField.height;
                    tmpx = (gameField.players[myID].col + Pacman::dx[fleeDir] + gameField.width) % gameField.width;
                    if (!(gameField.fieldStatic[gameField.players[myID].row][gameField.players[myID].col] &
                          Pacman::direction2OpposingWall[fleeDir]))
                        if (gameField.pathInfo[gameField.players[myID].row][gameField.players[myID].col].fleeLength ==
                            gameField.pathInfo[tmpy][tmpx].fleeLength + 1)
                            break;
                }
                for (auto i = tmpDangers.begin(); i != tmpDangers.end(); ++i)
                    if ((*i).first == fleeDir && (*i).second == tmpMax)
                    {
                        dir = fleeDir;
                        break;
                    }
            }

            return dir;
        }

        if (tmpDangers.size() == 4) //没得选
        {
            int k = 5; // -1 + 0 + 1 + 2 + 3
            for (auto i = tmpDangers.begin(); i != tmpDangers.end(); ++i)
                k -= (*i).first;
            return Pacman::Direction(k);
        }

        // Jet: 尝试发波
//        if (gameField.players[myID].strength >= gameField.SKILL_COST)
//        {
//            if ((gameField.Distance(myID, Helpers::MY_ID) >= 2 || Helpers::DeltaATK(gameField, Helpers::MY_ID, myID) <= 0)
//                && gameField.canDggbHit(myID, Helpers::MY_ID))
//            {
//                Pacman::Direction hitDir = gameField.hitDir;
//                std::vector<Pacman::Direction> valDir;
//                for (Pacman::Direction dir = Pacman::up; dir <= Pacman::left; ++dir)
//                {
//                    if (gameField.ActionValid(Helpers::MY_ID, dir))
//                        valDir.push_back(dir);
//                }
//                size_t s = valDir.size();
//                if ((s == 2 && (valDir[0] & 1 == valDir[1] & 1) && (valDir[0] & 1 == hitDir & 1))
//                    || (s == 1 && (valDir[0] & 1 == hitDir & 1)))
//                    return hitDir;
//            }
//            for (int _ = 0; _ < MAX_PLAYER_COUNT; ++_)
//            {
//                if (_ == myID || _ == Helpers::MY_ID)
//                    continue;
//                if ((gameField.Distance(myID, _) >= 2 || Helpers::DeltaATK(gameField, _, myID) <= 0)
//                    && gameField.canDggbHit(myID, _))
//                {
//                    Pacman::Direction hitDir = gameField.hitDir;
//                    std::vector<Pacman::Direction> valDir;
//                    for (Pacman::Direction dir = Pacman::up; dir <= Pacman::left; ++dir)
//                    {
//                        if (gameField.ActionValid(_, dir))
//                            valDir.push_back(dir);
//                    }
//                    size_t s = valDir.size();
//                    if ((s == 2 && (valDir[0] & 1 == valDir[1] & 1) && (valDir[0] & 1 == hitDir & 1))
//                        || (s == 1 && (valDir[0] & 1 == hitDir & 1)))
//                        return hitDir;
//                }
//            }
//        }

        //记录forbiddenDirs
        for (auto i = tmpDangers.begin(); i != tmpDangers.end(); ++i)
            forbiddenDirs |= (1 << ((*i).first + 1));

        //标记目标AI
        for (int _ = 0; _ < MAX_PLAYER_COUNT; _++)
        {
            Pacman::Player& rival = gameField.players[_];
            if (rival.dead || _ == myID)
                continue;
            if (Helpers::DeltaATK(gameField, myID, _) > 0)
            {
                bool preyFlag = gameField.pathInfo[rival.row][rival.col].isImpasse &&
                                (gameField.pathInfo[rival.row][rival.col].fleeLength + 2 >=
                                 gameField.Distance(gameField.players[myID],
                                                    *gameField.pathInfo[rival.row][rival.col].pExit));
                bool tryPreyFlag = gameField.pathInfo[rival.row][rival.col].isExit && gameField.Distance(myID, _) <= 2;

                if (preyFlag || tryPreyFlag)
                {
                    int tmpFruitCount = Helpers::fruitCount(gameField, gameField.players[_]);
                    //没有水果的死路一般不会走进去吧
                    if (tmpFruitCount >= Helpers::DeltaATK(gameField, myID, _) || tmpFruitCount == 0)
                        preyFlag = tryPreyFlag = false;
                }
                //夹道里被追击的弱AI
                if (!preyFlag && !tryPreyFlag)
                {
                    int dirCount = 4;
                    for (int i = 0; i < 4; ++i)
                    {
                        if (gameField.fieldStatic[rival.row][rival.col] & Pacman::direction2OpposingWall[i])
                        {
                            --dirCount;
                            continue;
                        }
                        Pacman::FieldProp checkPos;
                        checkPos.row = (rival.row + Pacman::dy[i] + gameField.height) % gameField.height;
                        checkPos.col = (rival.col + Pacman::dx[i] + gameField.width) % gameField.width;
                        if (gameField.fieldContent[checkPos.row][checkPos.col] & Pacman::playerMask)
                            for (int checkID = 0; checkID < 4; ++checkID)
                            {
                                if (checkID == myID)
                                    continue;
                                if (gameField.fieldContent[checkPos.row][checkPos.col] &
                                    Pacman::playerID2Mask[checkID] &&
                                    gameField.players[checkID].strength > rival.strength)
                                {
                                    --dirCount;
                                    break;
                                }
                            }
                    }
                    if (dirCount == 1 && gameField.Distance(myID, _) <= 2)
                        tryPreyFlag = true;
                }
                if (preyFlag)
                    playerTarget |= Pacman::playerID2Mask[_];
                if (tryPreyFlag)
                    tryPlayerTarget |= Pacman::playerID2Mask[_];
            }
        }

        auto&& fruitInfo = gameField.GetToTarget(myID, fruitTarget, forbiddenDirs);
        auto&& playerInfo = gameField.GetToTarget(myID, playerTarget, forbiddenDirs);
        auto&& tryPlayerInfo = gameField.GetToTarget(myID, tryPlayerTarget, forbiddenDirs);
        //一定概率放弃当前果子
        if (fruitInfo.dis == 0 && Helpers::RandBetween(0, 2) == 0)
            fruitInfo = gameField.GetToTarget(myID, fruitTarget, forbiddenDirs | 1);
#ifdef DEBUG
        //cout << '#' << myID << ' ' << (fruitInfo >> 3) << ' ' << Pacman::dirStr[fruitInfo & 7] << ' ' << (playerInfo >> 3) << ' ' << Pacman::dirStr[playerInfo & 7] << endl;
#endif // DEBUG
        auto fruitDirInfo = fruitInfo.dir;
        auto playerDirInfo = playerInfo.dir;
        auto tryPlayerDirInfo = tryPlayerInfo.dir;

        int info = (fruitDirInfo < 5) + ((tryPlayerDirInfo < 5) << 1) + ((playerDirInfo < 5) << 2);

        if (info >= 4)
            dir = playerDirInfo;
        else if (info >= 2)
            dir = tryPlayerDirInfo;
        else if (info >= 1 && fruitInfo.dis <= gameField.generatorTurnLeft)
            dir = fruitDirInfo;
        else
            dir = gameField.GetToNearbyGenerator(myID, forbiddenDirs).dir;

        if (dir != Pacman::Direction::stay && dir != Pacman::Direction::noPos)
            return dir;
        if (Helpers::RandBetween(0, 4) <= 2 && dir == Pacman::Direction::stay)
            return dir;
        else
            return Helpers::SimpleRandom(gameField, myID, forbiddenDirs);
    }

    int GreedyEval(Pacman::GameField& gameField, int myID)
    {
#ifdef PROFILING
        auto &&startTime = clock();
#endif
        int minMaxClusterDis = 100;
        int strengthSum = 0;
        if (gameField.players[myID].dead)
            return DEATH_EVAL;
        for (int i = 0; i < MAX_PLAYER_COUNT; ++i)
            strengthSum += gameField.players[i].strength;

        if (!gameField.hasNext)
        {
            int weakCount = 0;
            for (int i = 0; i < MAX_PLAYER_COUNT; ++i)
                if (gameField.players[i].strength < gameField.players[myID].strength)
                    ++weakCount;
            return int(1000 * float(gameField.players[myID].strength) / strengthSum + (weakCount + 1) * 100);
        }
        int e = 10;

        int strongCount = 0;
        for (int i = 0; i < MAX_PLAYER_COUNT; ++i)
            if (gameField.players[i].strength > gameField.players[myID].strength)
                ++strongCount;

        //e -= strongCount * 10;

        if (gameField.generatorCount == 0)
            minMaxClusterDis = 0;
        else
            minMaxClusterDis = gameField.GetToMaxCluster(myID).dis;

        if (minMaxClusterDis + 2 >= gameField.generatorTurnLeft)
            e -= (minMaxClusterDis + 2 - gameField.generatorTurnLeft) * 2;

        if (gameField.players[myID].powerUpLeft <= 0)
            e += gameField.players[myID].strength;
        else
            e += gameField.players[myID].strength -
                 gameField.LARGE_FRUIT_ENHANCEMENT; // +gameField.players[myID].powerUpLeft;
#ifdef PROFILING
        auto &&d = Debug::debugData["profiling"]["GreedyEval()"];
    d = d.asDouble() + double(clock() - startTime) / CLOCKS_PER_SEC;
#endif
        return e;
    }

    Solution chooseDir(Pacman::GameField& gameField, int myID, std::vector<std::vector<Solution>>& solutions,
                       std::vector<DangerInfoType> dangers)
    {
        int evalWeighedAverage[9]{};
        bool deathFlag[9]{};
        int tmp = 0;
        for (auto danger : dangers)
        {
            deathFlag[danger.first + 1] = true;
            evalWeighedAverage[danger.first + 1] = danger.second == 0 ? INVALID_EVAL : DEATH_EVAL + danger.second;
        }


        for (auto sol : solutions)
        {
            ++tmp;
            for (int i = 0; i < 5; i++)
            {
                if (sol[i].second == INVALID_EVAL)
                    continue;

                //这是为了分出最晚死的方向
                if ((sol[i].second <= (DEATH_EVAL + 100)) || deathFlag[i])
                {
                    if (!deathFlag[i])
                    {
                        deathFlag[i] = true;
                        evalWeighedAverage[i] = DEATH_EVAL + tmp;
                    }
                    continue;
                }
                evalWeighedAverage[i] += sol[i].second;
                evalWeighedAverage[i] /= 2;
            }
        }

        int average = 0;
        int count = 0;
        for (int i = 1; i <= 4; i++)
            if (evalWeighedAverage[i] > 0)
            {
                average += evalWeighedAverage[i];
                count++;
            }
        average /= count;

        //尝试放动感光波
        if (gameField.players[myID].strength > gameField.SKILL_COST)
            for (int _ = 0; _ < MAX_PLAYER_COUNT; ++_)
            {
                if (_ == myID || gameField.players[_].dead)
                    continue;
                if ((gameField.Distance(myID, _) >= 2 || Helpers::DeltaATK(gameField, _, myID) <= 0)
                    && gameField.canDggbHit(myID, _))
                {
                    Pacman::Direction hitDir = gameField.hitDir;
                    std::vector<Pacman::Direction> valDir;
                    for (Pacman::Direction dir = Pacman::up; dir <= Pacman::left; ++dir)
                    {
                        if (gameField.ActionValid(_, dir))
                            valDir.push_back(dir);
                    }
                    size_t s = valDir.size();
                    if (s == 2 && ((valDir[0] & 1) == (valDir[1] & 1)) && ((valDir[0] & 1) == (hitDir & 1)))
                        evalWeighedAverage[hitDir + 1] = average +
                                                         (gameField.players[_].strength >=
                                                          int(gameField.SKILL_COST * 1.5) ? int(
                                                                 gameField.SKILL_COST * 1.5) * 5 :
                                                          int(gameField.players[_].strength * 1.5) * 5);
                    else if (s == 1 && ((valDir[0] & 1) == (hitDir & 1)))
                        evalWeighedAverage[hitDir + 1] = average +
                                                         (gameField.players[_].strength >=
                                                          int(gameField.SKILL_COST * 1.5) ? int(
                                                                 gameField.SKILL_COST * 1.5) * 5 :
                                                          int(gameField.players[_].strength * 1.5) * 5);
                } else
                {
                    std::vector<Pacman::Direction> vDir;
                    for (Pacman::Direction dir = Pacman::up; dir <= Pacman::left; ++dir)
                    {
                        if (gameField.ActionValid(_, dir)
                            && gameField.canEat(_, dir)
                            && gameField.Distance(_, myID) >= 2)
                            vDir.push_back(dir);
                    }
                    if (vDir.size() == 1 && gameField.canDggbHitR(vDir[0], myID, _))
                    {
                        if (gameField.players[myID].strength >= 2 * gameField.SKILL_COST)
                            evalWeighedAverage[gameField.hitDir + 1] = average +
                                                                       (gameField.players[_].strength >=
                                                                        int(gameField.SKILL_COST * 1.5) * 3 ? int(
                                                                               gameField.SKILL_COST * 1.5) * 3 :
                                                                        int(gameField.players[_].strength * 1.5) * 3);
                    }
                }
            }
        //防止中波
        for (Pacman::Direction dir = Pacman::stay; dir <= Pacman::left; ++dir)
            for (int _ = 0; _ < MAX_PLAYER_COUNT; ++_)
            {
                bool flag = false;
                if (_ == myID)
                    continue;
                for (Pacman::Direction dir = Pacman::up; dir <= Pacman::left; ++dir)
                    if (gameField.canEat(_, dir))
                        flag += Helpers::RandBetween(0, 2);
                if (flag)
                    continue;
                if (gameField.players[_].strength > gameField.SKILL_COST
                    && (Helpers::DeltaATK(gameField, myID, _) <= gameField.SKILL_COST ||
                        gameField.Distance(myID, _) >= gameField.SKILL_COST / 2)
                    && gameField.canDggbHit(dir, myID, _))
                {
                    evalWeighedAverage[dir + 1] -= 3 * gameField.SKILL_COST;
                }
            }
        //避免进退两难的方向
        for (Pacman::Direction dir = Pacman::stay; dir <= Pacman::left; ++dir)
        {
            if (!gameField.ActionValid(myID, dir))
                continue;
            Pacman::FieldProp me = gameField.players[myID];
            if (dir != Pacman::stay)
            {
                me.row = (me.row + Pacman::dy[dir] + gameField.height) % gameField.height;
                me.col = (me.col + Pacman::dx[dir] + gameField.width) % gameField.width;
            }
            std::vector<Pacman::Direction> valDir;
            for (Pacman::Direction dir1 = Pacman::up; dir1 <= Pacman::left; ++dir1)
            {
                if (gameField.DirValid(me, dir1))
                    valDir.push_back(dir1);
            }
            size_t s = valDir.size();
            if ((s == 1) || (s == 2 && ((valDir[0] & 1) == (valDir[1] & 1))))
                evalWeighedAverage[dir + 1] -= gameField.SKILL_COST;
        }
        for (int i = 5; i < 9; i++)
            if (evalWeighedAverage[i] == 0)
                evalWeighedAverage[i] = evalWeighedAverage[0] - 4 * gameField.SKILL_COST;


        int max = INVALID_EVAL;
        auto d = Pacman::Direction::stay;
        for (int i = 0; i < 9; i++)
            if (max < evalWeighedAverage[i])
            {
                max = evalWeighedAverage[i];
                d = Pacman::Direction(i - 1);
                tmp = 1;
            } else if (max == evalWeighedAverage[i] && Helpers::RandBetween(0, ++tmp) == 0)
                d = Pacman::Direction(i - 1);

        for (int i = 0; i < 9; i++)
            Debug::debugData["#result"][Pacman::dirStr[i]] = evalWeighedAverage[i];

        return std::make_pair(d, max);
    }

// weaZen:简单的搜索，调用返回最高估值 若上一步造成力量变化则不给出lastDir
    int SimpleSearch(Pacman::GameField& gameField, int myID, int depth,
                     Pacman::Direction (*rivalAI)(Pacman::GameField&, int, int), Pacman::Direction lastDir,
                     std::vector<Solution>& solutions)
    {
        int max = DEATH_EVAL;
        int tmp = 0;
        int strength = gameField.players[myID].strength;
        int powerUpLeft = gameField.players[myID].powerUpLeft;
        //cout << depth << ' ';

        if (Debug::TimeOut() || depth == 0 || !gameField.hasNext)
            return GreedyEval(gameField, myID);
        if (gameField.players[myID].dead)
            return DEATH_EVAL;

        for (auto dir = Pacman::stay; dir <= Pacman::left; ++dir)
        {
            if (depth == maxDepth && solutions[dir + 1].second <= DEATH_EVAL)
                continue;
            if (!gameField.ActionValid(myID, dir))
            {
                if (depth == maxDepth)
                    solutions[dir + 1].second = INVALID_EVAL;
                continue;
            }
            if (Helpers::SimpleDangerJudge(gameField, myID, dir))
            {
                if (depth == maxDepth)
                    solutions[dir + 1].second = DEATH_EVAL;
                continue;
            }

            Pacman::FieldProp frontGrid;
            frontGrid.row = (gameField.players[myID].row + Pacman::dy[lastDir] + gameField.height) % gameField.height;
            frontGrid.col = (gameField.players[myID].col + Pacman::dx[lastDir] + gameField.width) % gameField.width;
            //基于以下两点猜测减少搜索量
            //1.没有力量增加或驱逐对手却往反方向跑是无意义的
            //2.不在生成器周围或当前位置没有果子却不动是无意义的
            if (lastDir != Pacman::Direction::stay && Pacman::dy[dir] + Pacman::dy[lastDir] == 0 &&
                Pacman::dx[dir] + Pacman::dx[lastDir] == 0 &&
                !(gameField.fieldContent[frontGrid.row][frontGrid.col] & Pacman::playerMask))
                continue;
            if (depth != maxDepth && dir == Pacman::Direction::stay &&
                (!gameField.genInfo[gameField.players[myID].row][gameField.players[myID].col].isBesideGen ||
                 gameField.generatorTurnLeft > 3) &&
                !(gameField.fieldContent[gameField.players[myID].row][gameField.players[myID].col] &
                  (Pacman::GridContentType::smallFruit | Pacman::GridContentType::largeFruit)))
                continue;

            for (int i = 0; i < MAX_PLAYER_COUNT; i++)
            {
                if (i == myID)
                    continue;
                if (gameField.players[i].dead)
                    continue;
                gameField.actions[i] = rivalAI(gameField, i, myID);
            }

            gameField.actions[myID] = dir;

#ifdef DEBUG
            if (depth == maxDepth && myID == 0)
        {
            //gameField.DebugPrint();
            for (int i = 0; i < MAX_PLAYER_COUNT; ++i)
            {
                cout << "AI " << i << ' ' << Pacman::dirStr[gameField.actions[i] + 1] << endl;
            }
            //system("pause");
        }
#endif // DEBUG

            gameField.NextTurn();

            //多个玩家重叠在果子上允许返回
            if (gameField.players[myID].strength - strength == 0 &&
                !(gameField.fieldContent[gameField.players[myID].row][gameField.players[myID].col] &
                  (Pacman::GridContentType::smallFruit | Pacman::GridContentType::largeFruit)))
            {
                if (dir == Pacman::stay)
                    tmp = SimpleSearch(gameField, myID, depth - 1, rivalAI, lastDir, solutions);
                else
                    tmp = SimpleSearch(gameField, myID, depth - 1, rivalAI, dir, solutions);
            } else
                tmp = SimpleSearch(gameField, myID, depth - 1, rivalAI, Pacman::stay, solutions);

            //不在死路上吃到了敌人 因为有风险先还原再说
            if (gameField.players[myID].strength - strength > 1 &&
                gameField.players[myID].powerUpLeft - powerUpLeft != 9 &&
                !gameField.pathInfo[gameField.players[myID].row][gameField.players[myID].col].isImpasse &&
                !gameField.pathInfo[gameField.players[myID].row][gameField.players[myID].col].isExit)
                tmp = tmp - (gameField.players[myID].strength - strength) * depth + 1;

            //吃到大果子稍微加一分
            if (gameField.players[myID].strength - strength > 1 &&
                gameField.players[myID].powerUpLeft - powerUpLeft == 9)
                tmp += gameField.largeFruitValue;

            gameField.RollBack(1);

            if (tmp > 0 && depth + gameField.turnID != MAX_TURN)
                tmp += GreedyEval(gameField, myID);
            if (depth == maxDepth && tmp > 0 && dir == Pacman::stay &&
                !(gameField.fieldContent[gameField.players[myID].row][gameField.players[myID].col] &
                  (Pacman::GridContentType::smallFruit | Pacman::GridContentType::largeFruit)) &&
                gameField.players[myID].strength - strength == 0)
                tmp = int(tmp * (1 - float(gameField.generatorTurnLeft - 1) / gameField.GENERATOR_INTERVAL));
            if (tmp > max)
                max = tmp;
            if (depth == maxDepth)
            {
                // Jet: ???
//                if (tmp <= DEATH_EVAL)
//                    solutions[dir + 1].second = tmp;
//                else
                solutions[dir + 1].second = tmp;
            }

            // 超时处理
            if (Debug::TimeOut())
                return max;
        }

        return max;
    }

    Pacman::Direction IterativeGreedySearch(Pacman::GameField& gameField, int myID)
    {
        std::vector<Solution> sol(5);
        std::vector<DangerInfoType> dangers(5);
        for (int i = 0; i < 5; i++)
        {
            sol[i].first = Pacman::Direction(i - 1);
            sol[i].second = 0;
        }

        std::vector<std::vector<Solution>> solutions{};

        DangerJudge(gameField, myID, dangers);

        for (auto danger : dangers)
        {
            sol[danger.first + 1].second = danger.second == 0 ? INVALID_EVAL : DEATH_EVAL;
        }

        for (int depth = DEFAULT_DEPTH; depth <= std::min(MAX_DEPTH, 100 - gameField.turnID); depth++)
        {
            auto startTime = clock();
            maxDepth = depth;

            SimpleSearch(gameField, myID, depth, NaiveThinkAI, Pacman::stay, sol);
            if (Debug::TimeOut())
            {
                Debug::debugData[Helpers::depth2String(depth)]["*solution"]["notFinished"] = true;
                break;
            }

            solutions.push_back(sol);
            for (auto i = Pacman::stay; i <= Pacman::left; ++i)
                Debug::debugData[Helpers::depth2String(depth)]["evals"][Pacman::dirStr[i + 1]] = sol[i + 1].second;
            Debug::debugData[Helpers::depth2String(depth)]["timeCosumed"] =
                    double(clock() - startTime) / CLOCKS_PER_SEC;
        }

        cout << endl;
        if (solutions.size() == 0)
        {
            Debug::debugData["*choice"]["NAIVE"] = true;
            return NaiveAttackAI(gameField, myID, -1);
        }

        auto&& finalSol = chooseDir(gameField, myID, solutions, dangers);

        Debug::debugData["*choice"]["direction"] = Pacman::dirStr[finalSol.first + 1];
        Debug::debugData["*choice"]["finalEval"] = finalSol.second;

        return finalSol.first;
    }
}

int main()
{
    auto AI = AI::IterativeGreedySearch;

    Pacman::GameField mainGameField;
    Json::Value data, globalData; // 这是回合之间可以传递的信息
    // 如果在本地调试，有input.txt则会读取文件内容作为输入
    // 如果在平台上，则不会去检查有无input.txt
#ifdef _BOTZONE_ONLINE
    Debug::presetString.clear();
#endif

    int myID = mainGameField.ReadInput("input.txt", data, globalData); // 输入，并获得自己ID
    Helpers::MY_ID = myID;
    srand(unsigned(Pacman::seed + myID));

    // 输出当前游戏局面状态以供本地调试。注意提交到平台上会自动优化掉，不必担心。
    mainGameField.DebugPrint();

#ifndef _BOTZONE_ONLINE
    Debug::startTime = clock();
    Debug::printInfo = true;
#endif

    // 中央决定一定要卖萌
    auto&& choice = AI(mainGameField, myID);
    Debug::debugData["profiling"]["TimeUsed"] = Debug::TimeThrough();
    string taunt = ""; // = choice == Pacman::stay ? "吓得本宝宝不敢动 TAT" : TAUNT();
    mainGameField.WriteOutput(choice, taunt, data, globalData, Debug::debugData);

#ifndef _BOTZONE_ONLINE
    system("pause");
#endif
    return 0;
}