#ifndef HOCKEY_DEFINITIONS_HPP_68F5DE9D_8E28_4357_AC70_B8FF22ED8377
#define HOCKEY_DEFINITIONS_HPP_68F5DE9D_8E28_4357_AC70_B8FF22ED8377

/*----------------*
 * 寸法は全て[mm] *
 *----------------*/

/* ホッケー台 */
const double HKY_W = 1407.75;  // 横
const double HKY_H = 2288.975; // 縦
/* ホッケーアーム */
const double ARM_OFFSET = 211.0125; // 台の壁からアーム始点までの長さ
const double ARM_1LEN  = 558.5;     // 腕(モーター側)の長さ
const double ARM_2LEN  = 319.45;    // 腕(マレット側)の長さ
const double ARM_MLT_R = 100.0;  // マレットの直径
/* その他 */
const double PUCK_R = 85.0;        // パックの直径
const double PLAYER_MLT_R = 100.0; // プレイヤー側のマレットの直径

/* アーム系の設定値 */
const int ENCODER_EVAL  =  1; // エンコーダカウントの逓倍数
const int MOTOR_X_RATIO = 25; // 肩モーターの減速比
const int MOTOR_Y_RATIO = 15; // 肘モーターの減速比

//const int SLOW_LIMIT_X = 10000; // 減速し始めるカウント数(X軸モーター)
//const int SLOW_LIMIT_Y =  7500; // 減速し始めるカウント数(Y軸モーター)
//const int SLOW_LIMIT_X = 5000; // 減速し始めるカウント数(X軸モーター)
//const int SLOW_LIMIT_Y = 5000; // 減速し始めるカウント数(Y軸モーター)
const int SLOW_LIMIT_X = 7500; // 減速し始めるカウント数(X軸モーター)
const int SLOW_LIMIT_Y = 7500; // 減速し始めるカウント数(Y軸モーター)

//const int INIT_ENC_X   = 2388;
//const int INIT_ENC_Y   = 1593;
//const int INIT_ENC_X = 1296;
//const int INIT_ENC_Y = 1607;//-100;

//const int INIT_ENC_X = 266;
//const int INIT_ENC_Y = 777;

const int INIT_ENC_X = 225;
const int INIT_ENC_Y = 965;

#endif // HOCKEY_DEFINITIONS_HPP
