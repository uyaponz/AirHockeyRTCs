#!/usr/bin/env python
# -*- coding: utf-8 -*-

### TODO : エラーチェック処理を入れる ###

import sys
import commands

from omniORB import CORBA
import OpenRTM_aist, RTC


# 端末のパスを取得する
def get_terminal_path() :
    status=0 ; term=""
    for name in get_terminal_path.termlist :
        status,term = commands.getstatusoutput("which %s"%name)
        if status == 0 : break

    if status != 0 :
        sys.stderr.write("No terminal program exists.\n")
        return None
    return term
get_terminal_path.termlist = ("kterm", "xterm", "uxterm", "gnome-terminal")


# RTC関連の便利関数
def get_component_object_rtc(corba_naming, rtcname) :
    comp = OpenRTM_aist.CorbaConsumer()
    comp.setObject(corba_naming.resolve(rtcname))
    return comp.getObject()._narrow(RTC.RTObject)
def get_eclist_rtc(comp_obj) :
    return comp_obj.get_owned_contexts()


# 指定した名前のRTCが存在するか(True/False)
def is_running_rtc(corba_naming, rtcname) :
    if corba_naming.resolve(rtcname) == None : return False
    return True


# コンポーネントが動作(Start)しているか(True/False)
def is_starting_rtc(corba_naming, rtcname) :
    comp_obj = get_component_object_rtc(corba_naming, rtcname)
    eclist = get_eclist_rtc(comp_obj)
    return eclist[0].is_running()


# コンポーネントの状態を取得する(LifeCycleState)
def get_state_rtc(corba_naming, rtcname) :
    comp_obj = get_component_object_rtc(corba_naming, rtcname)
    eclist = get_eclist_rtc(comp_obj)
    return eclist[0].get_component_state(comp_obj)


# コンポーネントの(de)activate
def set_active_rtc(corba_naming, rtcname) :
    comp_obj = get_component_object_rtc(corba_naming, rtcname)
    eclist = get_eclist_rtc(comp_obj)
    return eclist[0].activate_component(comp_obj)
def set_deactive_rtc(corba_naming, rtcname) :
    comp_obj = get_component_object_rtc(corba_naming, rtcname)
    eclist = get_eclist_rtc(comp_obj)
    return eclist[0].deactivate_component(comp_obj)

# コンポーネントのStart/Stop
def set_start_rtc(corba_naming, rtcname) :
    comp_obj = get_component_object_rtc(corba_naming, rtcname)
    eclist = get_eclist_rtc(comp_obj)
    return eclist[0].start(comp_obj)
def set_stop_rtc(corba_naming, rtcname) :
    comp_obj = get_component_object_rtc(corba_naming, rtcname)
    eclist = get_eclist_rtc(comp_obj)
    return eclist[0].stop(comp_obj)


# ポート間を接続する
def connect_ports_rtc(corba_naming,
                      rtcname_1,  portname_1,
                      rtcname_2, portname_2) :
    comp_1    = get_component_object_rtc(corba_naming, rtcname_1)
    comp_1_p  = comp_1.get_ports()
    comp_2   = get_component_object_rtc(corba_naming, rtcname_2)
    comp_2_p = comp_2.get_ports()

    # IN側のポート検索
    port_1 = None
    for p in comp_1_p :
        if p.get_port_profile().name == portname_1 :
            port_1 = p
            break
    if port_1 == None : return None

    # OUT側のポート検索
    port_2 = None
    for p in comp_2_p :
        if p.get_port_profile().name == portname_2 :
            port_2 = p
            break
    if port_2 == None : return None

    # ポート接続の設定値
    profile_name = port_1.get_port_profile().name + "_" + port_2.get_port_profile().name
    prof_properties = [
        ["dataport.interface_type"       , "corba_cdr"],
        ["dataport.dataflow_type"        , "push"     ],
        ["dataport.subscription_type"    , "flush"    ]]

    # 接続する
    conprof = RTC.ConnectorProfile(profile_name, "", [port_1, port_2], [])
    for prop in prof_properties :
        OpenRTM_aist.CORBA_SeqUtil.push_back(conprof.properties,
                                             OpenRTM_aist.NVUtil.newNV(prop[0], prop[1]))
    ret,conprof = port_1.connect(conprof)
    return ret


# ポートの接続を切断する
def disconnect_port_rtc(corba_naming, rtcname, portname) :
    comp_obj = get_component_object_rtc(corba_naming, rtcname)
    comp_p = comp_obj.get_ports()
    for p in comp_p :
        if p.get_port_profile().name == portname :
            return p.disconnect_all()
    return None


if __name__ == "__main__" :
    pass
