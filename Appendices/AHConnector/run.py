#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys, os
import time
import myfunc
from omniORB import CORBA
import OpenRTM_aist, RTC
import myfunc


rtchost = "localhost:9876"

# procs : (コンポーネント名, カレントディレクトリ, 起動コマンド)
procs = (
    ("AHLogger0.rtc",        "/home/shared/Projects/Releases/AHComp/AHLoggerComp",        "./AHLoggerComp"),
    ("AHViewer0.rtc",        "/home/shared/Projects/Releases/AHComp/AHViewerComp",        "./AHViewerComp"),
    ("HockeyArm0.rtc",       "/home/shared/Projects/Releases/AHComp/HockeyArmComp",       "./HockeyArmComp"),
    ("HockeyArmFK0.rtc",     "/home/shared/Projects/Releases/AHComp/HockeyArmFKComp",     "./HockeyArmFKComp"),
    ("HockeyArmIK0.rtc",     "/home/shared/Projects/Releases/AHComp/HockeyArmIKComp",     "./HockeyArmIKComp"),
    ("MalletTracker0.rtc",   "/home/shared/Projects/Releases/AHComp/MalletTrackerComp",   "./MalletTrackerComp"),
    ("PuckTrackerConv0.rtc", "/home/shared/Projects/Releases/AHComp/PuckTrackerConvComp", "./PuckTrackerConvComp"),
    ("pack_tracker0.rtc",    "/home/shared/Projects/Releases/AHComp/pack_tracker",        "./pack_trackerComp"),
    ("AHAttack0.rtc",        "/home/shared/Projects/Releases/AHComp/AHAttackComp",        "./AHAttackComp"),
    ("AHstop0.rtc",          "/home/admin/MidoHockeyRTC/AHstopComp/AHstopComp_1124",      "./AHstopComp"),
    ("LSQ_NUM0.rtc",         "/home/admin/MidoHockeyRTC/LSQ/LSQ_NUM_1124",                "./LSQ_NUMComp")
    )

# connects : (コンポーネント名1, ポート名1, コンポーネント名2, ポート名2)
connects = (
    ( # 最低限必要なもの
        ("pack_tracker0.rtc",    "pack_tracker0.pack_pos",
         "PuckTrackerConv0.rtc", "PuckTrackerConv0.PuckTrackerData"),
        ("HockeyArmIK0.rtc",     "HockeyArmIK0.ArmTh_Out",
         "HockeyArm0.rtc",       "HockeyArm0.MoveTh"),
        ("HockeyArm0.rtc",       "HockeyArm0.RealTh",
         "HockeyArmFK0.rtc",     "HockeyArmFK0.ArmTh_In")),
    ( # ロガー関連
        ("PuckTrackerConv0.rtc", "PuckTrackerConv0.PuckXY",
         "AHLogger0.rtc",        "AHLogger0.PuckXY"),
        ("MalletTracker0.rtc",   "MalletTracker0.MalletXY_out",
         "AHLogger0.rtc",        "AHLogger0.PlayerMalletXY"),
        ("HockeyArmFK0.rtc",     "HockeyArmFK0.MalletXY_Out",
         "AHLogger0.rtc",        "AHLogger0.RobotMalletXY")),
    ( # ビューア関連
        ("PuckTrackerConv0.rtc", "PuckTrackerConv0.PuckXY",
         "AHViewer0.rtc",        "AHViewer0.PuckXY_in"),
        ("MalletTracker0.rtc",   "MalletTracker0.MalletXY_out",
         "AHViewer0.rtc",        "AHViewer0.PMalletXY_in"),
        ("HockeyArmFK0.rtc",     "HockeyArmFK0.MalletXY_Out",
         "AHViewer0.rtc",        "AHViewer0.RMalletXY_in"))
    )
# activate_list : (コンポーネント名1, 2, ...)
activate_list = (
    ( # 常に起動しておくもの
        "AHViewer0.rtc",
        "HockeyArmFK0.rtc", "HockeyArmIK0.rtc",
        "MalletTracker0.rtc",
        "pack_tracker0.rtc", "PuckTrackerConv0.rtc"
        ),
    ( # 実験時に起動するもの(戦略RTC以外)
        "HockeyArm0.rtc",
        "AHLogger0.rtc"
        )
    )


# atkcon : 対戦アルゴリズムのconnects
atkcon = (
    ( # AHAttack
        ("PuckTrackerConv0.rtc", "PuckTrackerConv0.PuckXY",
         "AHAttack0.rtc",        "AHAttack0.PuckXY"),
        ("AHAttack0.rtc",        "AHAttack0.ArmXY",
         "HockeyArmIK0.rtc",     "HockeyArmIK0.MalletXY_In"),
        ("HockeyArmFK0.rtc",     "HockeyArmFK0.MalletXY_Out",
         "AHAttack0.rtc",        "AHAttack0.RealArmXY")),
    ( # LSQ_NUM, AHstop
        ("PuckTrackerConv0.rtc", "PuckTrackerConv0.PuckXY",
         "LSQ_NUM0.rtc",         "LSQ_NUM0.PuckIn"),
        ("AHstop0.rtc",          "AHstop0.Target",
         "HockeyArmIK0.rtc",     "HockeyArmIK0.MalletXY_In"),
        ("HockeyArmFK0.rtc",     "HockeyArmFK0.MalletXY_Out",
         "AHstop0.rtc",          "AHstop0.Arm"),
        ("LSQ_NUM0.rtc",         "LSQ_NUM0.PuckOut",
         "AHstop0.rtc",          "AHstop0.Puck")),
    ( # Imitation_Counter
        ("PuckTrackerConv0.rtc",   "PuckTrackerConv0.PuckXY",
         "Imitation_Counter0.rtc", "Imitation_Counter0.PuckXY_in"),
        ("Imitation_Counter0.rtc", "Imitation_Counter0.RMalletXY_out",
         "HockeyArmIK0.rtc",       "HockeyArmIK0.MalletXY_In")),
    ( # Imitation_Cut
        ("PuckTrackerConv0.rtc",   "PuckTrackerConv0.PuckXY",
         "Imitation_Cut0.rtc", "Imitation_Cut0.PuckXY_in"),
        ("Imitation_Cut0.rtc", "Imitation_Cut0.RMalletXY_out",
         "HockeyArmIK0.rtc",       "HockeyArmIK0.MalletXY_In")),
    ( # TEMPLATE!!
        ("PuckTrackerConv0.rtc", "PuckTrackerConv0.PuckXY",
         "", ""),
        ("MalletTracker0.rtc", "MalletTracker0.MalletXY_out",
         "", ""),
        ("", "",
         "HockeyArmIK0.rtc", "HockeyArmIK0.MalletXY_In"),
        ("HockeyArmFK0.rtc", "HockeyArmFK0.MalletXY_Out",
         "", ""))
    )
# atkactivate_list : 対戦アルゴリズムのactivate_list
atkactivate_list = (
    ( # AHAttack
        "AHAttack0.rtc", "AHAttack0.rtc"
        ),
    ( # LSQ_NUM, AHstop
        "LSQ_NUM0.rtc", "AHstop0.rtc"
        ),
    ( # Imitation_Counter
        "Imitation_Counter0.rtc", "Imitation_Counter0.rtc"
        ),
    ( # Imitation_Cut
        "Imitation_Cut0.rtc", "Imitation_Cut0.rtc"
        )
    )


def connect_components(corba_naming, connect_list) :
    for l in connect_list :
        if myfunc.connect_ports_rtc(corba_naming, l[0], l[1], l[2], l[3]) == None :
            print "error : cannot connect -", l
    return
def disconnect_components(corba_naming, connect_list) :
    for l in connect_list :
        if myfunc.disconnect_port_rtc(corba_naming, l[0], l[1]) == None :
            print "error : cannot disconnect -", l
    return
def activate_components(corba_naming, rtc_list) :
    for l in rtc_list :
        myfunc.set_active_rtc(corba_naming, l)
    return
def deactivate_components(corba_naming, rtc_list) :
    for l in rtc_list :
        myfunc.set_deactive_rtc(corba_naming, l)
    return

def main() :
    term = myfunc.get_terminal_path()
    if term == None : sys.exit(1)

    # OpenRTMの初期化
    orb = CORBA.ORB_init(sys.argv)
    naming = OpenRTM_aist.CorbaNaming(orb, rtchost)

    # コンポーネントを起動する
    # 例) os.system("%s -e python ConsoleIn.py &" % term)
    for p in procs :
        if myfunc.is_running_rtc(naming, p[0]) == False :
            os.system("%s -e 'cd %s && %s' &" %(term, p[1], p[2]))

    # ちょっと待つ
    time.sleep(0.5)
    raw_input("[press enter] to check running status...")

    # コンポーネントの起動確認
    for p in procs :
        if myfunc.is_running_rtc(naming, p[0]) == False :
            print "running? (%s)" % p[0]
    raw_input("[press enter] to connect components...")

    # 対戦RTC以外のコンポーネントをつなぐ
    connect_components(naming, connects[0]) # 最低限必要なもの
    connect_components(naming, connects[1]) # ロガー
    connect_components(naming, connects[2]) # ビューア
    time.sleep(0.5)
    activate_components(naming, activate_list[0])
    print "connect - COMPLETE!!!"

    # アルゴリズムの切り替えループ
    algo = -1
    while True :
        print "<コマンド一覧>"
        print "\t", "q:   プログラムの終了"
        print "\t", "0-9: アルゴリズムの切り替え"
        print "\t", "s:   対戦開始"
        print "\t", "e:   対戦終了"
        print "---"
        print "<アルゴリズム一覧>"
        print "\t", "0 : 単純な打ち返し(AHAttack)"
        print "\t", "1 : 止め打ち(LSQ_NUM, AHstop)"
        print "\t", "2 : 学習済みアルゴリズム(カウンター打ち, Imitation_Counter)"
        print "\t", "3 : 学習済みアルゴリズム(カット打ち, Imitation_Cut)"
        cmd = raw_input("input command > ")

        if cmd == 'q' :
            if algo != -1 :
                deactivate_components(naming, activate_list[1])
                deactivate_components(naming, atkactivate_list[algo])
                time.sleep(0.5)
                disconnect_components(naming, atkcon[algo])
            break
        elif cmd == 's' :
            if algo != -1 :
                activate_components(naming, atkactivate_list[algo])
                activate_components(naming, activate_list[1])
        elif cmd == 'e' :
            if algo != -1 :
                deactivate_components(naming, activate_list[1])
                deactivate_components(naming, atkactivate_list[algo])
        elif cmd.isdigit() :
            if (int(cmd) >= 0) and (int(cmd) <= 3) :
                deactivate_components(naming, activate_list[1])
                deactivate_components(naming, activate_list[0])

                if algo != -1 :
                    deactivate_components(naming, atkactivate_list[algo])
                    time.sleep(0.3)
                    disconnect_components(naming, atkcon[algo])
                else : time.sleep(0.3)

                algo = int(cmd)
                connect_components(naming, connects[0]) # 最低限必要なもの
                connect_components(naming, connects[1]) # ロガー
                connect_components(naming, connects[2]) # ビューア
                connect_components(naming, atkcon[algo])

                time.sleep(0.3)
                activate_components(naming, activate_list[0])

    # 対戦RTC以外のコンポーネントの接続を解除する
    print "disconnect start!"
    deactivate_components(naming, activate_list[1])
    deactivate_components(naming, activate_list[0])
    time.sleep(0.5)
    disconnect_components(naming, connects[0]) # 最低限必要なもの
    disconnect_components(naming, connects[1]) # ロガー
    disconnect_components(naming, connects[2]) # ビューア
    print "disconnect - COMPLETE!!!"

    return

if __name__ == "__main__" :
    main()
