<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="UTF-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, user-scalable=no, initial-scale=1">
    <title>控制台</title>
    <link rel="stylesheet" type="text/css" href="./mqtt_client/index.css">
</head>
<body>


<div class="slidiedoor">
    <!-- -------------内容块------------- -->

    <div class="container">
        <!-- ----------顶部---------------
        <div class="sidetop">
            <div class="side_door">
                <i class="iconfont">&#xe613;</i>
                <span></span>
            </div>
        </div>
 -->
        <!-- ----------状态--------------- -->
		<div class="conn-indicator"></div>
		</div>
        <div class="slid_status">
            <div class="stae">
                <div class="stac">
                    <div class="myst">
                        <div><i class="iconfont">&#xe62b;</i></div>
                        <div class="st_line">
                            <span>设备状态：</span>
                            <span class="s-status">正常</span>
                        </div>
                    </div>
                </div>
                <div class="stac">
                    <div class="myst">
                        <div><i class="iconfont">&#xe615;</i></div>
                        <div class="st_line">
                            <span>通信状态：</span>
                            <span class="c-status">正常</span>
                        </div>
                    </div>
                </div>
            </div>
            <div class="stae">
                <div class="stac">
                    <div class="myst">
                        <div><i class="iconfont">&#xec12;</i></div>
                        <div class="st_line">
                            <span>温度：</span>
                            <span class="t-degree">20℃</span>
                        </div>
                    </div>
                </div>
                <div class="stac">
                    <div class="myst">
                        <div><i class="iconfont">&#xe9a4;</i></div>
                        <div class="st_line">
                            <span>湿度：</span>
                            <span class="w-degree">100%</span>
                        </div>
                    </div>
                </div>
            </div>
            <div class="stae">
                <div class="stac">
                    <div class="myst">
                        <div><i class="iconfont">&#xe6b6;</i></div>
                        <div class="st_line">
                            <span>光照度：</span>
                            <!--<span class="p-rate">100%</span>!-->
                            <span class="r-degree">100%</span>
							
                        </div>
                    </div>
                </div>
                <div class="stac">
                    <div class="myst">
                        <div><i class="iconfont">&#xe663;</i></div>
                        <div class="st_line">
                            <span>运行时间：</span>
                            <span class="r-time">12:01:20</span>
                        </div>
                    </div>
                </div>
            </div>
        </div>

        <!-- -------------操控-------------- -->
        <div class="side_control">
            <div class="side_c_top">
                <div class="side_c">
                    <i class="iconfont">&#xe60a;</i>
                    <span>操控</span>
                </div>
            </div>
            <div class="side_btn">
                <ul>
                    <li class="op-control-closed left" data-type="0">Rely1</li>
                    <li class="op-control-closed" data-type="1">Rely2</li>
                    <li class="op-control-closed" data-type="2">Rely3</li>
                    <li class="op-control-opened" data-type="3">Rely4</li>
                </ul>
            </div>
			</br>
			<div class="side_c">
			<!-----xe6b9----->
				<i class="iconfont">&#xe62a;</i>
				<span>可调灯</span>
			</div>
			<section style="margin-top:30px;margin-bottom:30px">
				<div id="js-example-change-value" >
				   <input id="lightDutyValue" type="range" min="0" max="100" value="1" data-rangeslider>
				</div>
			</section>
			<div class="side_c">
			<!-----xe6b9----->
				<i class="iconfont">&#xe62a;</i>
				<span>光强触发值</span>
			</div>
			<section style="margin-top:30px;margin-bottom:30px">
			<div class="rayAlarmValueCt">
			   <input id="rayAlarmValue" type="range" min="0" max="100" value="1" data-rangeslider>
			</div>
			</section>
			<div class="side_c">
			<!-----xe6b9----->
				<i class="iconfont">&#xe62a;</i>
				<span>步进电机</span>
			</div>
			<div class="side_c">
			<div class="side_btn motorCtl_btn">
                <ul>
                    <li class="op-motor" data-type="0">CW</li>
                    <li class="op-motor" data-type="1">Stop</li>
                    <li class="op-motor" data-type="2">CCW</li>
                </ul>
            </div>
			</div>
        </div>
        <div class="peiz">
            <!-- -------------是否可读可写--------------- -->
            <div class="write">
                <div class="deploy_title">
                    <div class="wr_tit">
                        <i class="iconfont">&#xe619;</i>
                        <span>锁定配置</span>
                    </div>
                    <label class="switch">
                        <input type="checkbox" id="writer_ch">
                        <div class="slider round"></div>
                    </label>
                </div>
            </div>

            <!-- -------------认证信息--------------- -->
            <div class="deploy dtoken">
                <div class="deploy_title pda">
                    <div>
                        <i class="iconfont">&#xe64b;</i>
                        <span>认证信息</span>
                    </div>
                    <i class="iconfont dep_t_icon">&#xe650;</i>
                </div>
                <div class="deploy_token deploy_con" style="display:none;">
                    <div class="dt_title">
                        <div>
                            <span>唯一标识符：</span>
                        </div>
                        <!--<i class="iconfont dep_t_icon bian">&#xe68e;</i>-->
                    </div>
                    <div class="dt_con">
                        <p class="s-token">D2F0D552-89DC-0434-2E0B-83DAAD141890</p>
                    </div>
                    <div class="dt_input" style="display:none;">
                        <input type="text" value="D2F0D552-89DC-0434-2E0B-83DAAD141890">
                        <div class="dt_input_btn">
                            <button class="cancel">取消</button>
                            <button class="confi">确定</button>
                        </div>
                    </div>
                </div>
            </div>

            <!-- -------------WIFI配置--------------- -->
            <div class="deploy" id="wificonfig">
                <div class="deploy_title">
                    <div>
                        <i class="iconfont">&#xe61e;</i>
                        <span>WIFI配置</span>
                    </div>
                    <i class="iconfont dep_t_icon">&#xe650;</i>
                </div>
                <div class="deploy_con" style="display:none;">
                    <div class="deploy_i01">
                        <!--隐藏域-->
                        <input type="hidden" value="1" class="w-mode">
                        <div class="i01_l">
                            <i class="iconfont">&#xe6cf;</i>
                            <span>工作模式</span>
                        </div>
                        <div class="i02_r">
                            <div class="i02_r01 mode">
                                <span class="r01 active" data-work_mode="1"></span>本地
                            </div>
                            <div class="i02_r02 mode">
                                <span class="r01" data-work_mode="0"></span>云端
                            </div>
                            <div class="clearfix"></div>
                        </div>
                    </div>
                    <div class="deploy_con_m">
                        <div class="d_c_m">
                            <div>
                                <i class="iconfont">&#xe605;</i>
                                <label>WIFI账号(本地)：</label>
                            </div>
                            <div class="ipd">
                                <input type="text" class="wa-ssid" placeholder="请输入本地WIFI账号">
                            </div>
                        </div>
                        <div class="d_c_m">
                            <div>
                                <i class="iconfont">&#xe626;</i>
                                <label>WIFI密码(本地)：</label>
                            </div>
                            <div class="ipd">
                                <input type="password" class="wa-pwd" placeholder="请输入本地WIFI密码">
                            </div>
                        </div>
                    </div>
                    <div class="deploy_con_m">
                        <div class="d_c_m">
                            <div>
                                <i class="iconfont">&#xe605;</i>
                                <label>WIFI账号(云端)：</label>
                            </div>
                            <div class="ipd">
                                <input type="text" class="ws-ssid" placeholder="请输入云端WIFI账号">
                            </div>
                        </div>
                        <div class="d_c_m">
                            <div>
                                <i class="iconfont">&#xe626;</i>
                                <label>WIFI密码(云端)：</label>
                            </div>
                            <div class="ipd">
                                <input type="password" class="ws-pwd" placeholder="请输入云端WIFI密码">
                            </div>
                        </div>
                    </div>
                    <div class="ds">
                        <div class="deploy_save wifiscan-btn">扫描</div>
                    </div>
                    <div class="ds">
                        <div class="deploy_save w-btn">保存配置</div>
                    </div>
                </div>
            </div>

            <!-- -------------门配置--------------- -->
            <div class="deploy">
                <div class="deploy_title">
                    <div>
                        <i class="iconfont">&#xe607;</i>
                        <span>配置</span>
                    </div>
                    <i class="iconfont dep_t_icon">&#xe650;</i>
                </div>
				
                <div class="deploy_con"  style="display:none;">
				
                    <div class="deploy_i03">
                        <div class="deploy_i02 i032">
                            <div class="i01_l">
                                <i class="iconfont">&#xe614;</i>
                                <span>Test</span>
                            </div>
                            <div>
                                <label class="switch" >
                                    <input type="checkbox" class="is_external_control_enabled" />
                                    <div class="slider round"></div>
                                </label>
                            </div>
                        </div>
                    </div>
                    <!-- -------------选择方案--------------- -->
                    <div class="plan">
                        <div class="plan_ic">
                            <i class="iconfont">&#xe63c;</i>
                            <span>Choice</span>
                        </div>
                        <div class="plan_sel">
                            <div class="plan_btn" data-status="0">
                                <span>不使用</span>
                                <i class="iconfont">&#xe671;</i>
                            </div>
                            <ul class="plan_ul" style="display: none;">
                                <li class="speed" data-index="-1">不使用</li>
                                <li class="speed" data-index="0">10kg以内</li>
                                <li class="speed" data-index="1">15kg - 35kg</li>
                                <li class="speed" data-index="2">40kg - 60kg</li>
                                <li class="speed" data-index="3">65kg - 85kg</li>
                                <li class="speed" data-index="4">90kg - 110kg</li>
                                <li class="speed" data-index="5">115kg - 125kg</li>
                            </ul>
                        </div>
                    </div>
                    

                    <div class="ds">
                        <div class="deploy_save d-btn">保存配置</div>
                    </div>
                </div>
            </div>
        </div>

        <!-- -------------工程参数--------------- -->
        <div class="enginer">
            <!-- 参数是否可改 -->
            <div class="write">
                <div class="deploy_title">
                    <div class="wr_tit">
                        <i class="iconfont">&#xe619;</i>
                        <span>测试2</span>
                    </div>
                    <label class="switch" id="in_enginer" data-status="0">
                        <input type="checkbox">
                        <div class="slider1 round"><span></span></div>
                    </label>
                </div>
            </div>
            <!-- 参数 -->
        </div>
		
<!-- -----------对话框------------ -->
<div class="en_diak config_frame" style="display: none;">
    <div class="enda">
        <div class="endi_title">配置密码</div>
        <div class="en_mima">
            <input type="password" class="p-password" placeholder="请输入配置密码">
        </div>
        <div class="endia_btn">
            <div class="en_confirm">确定</div>
            <div class="en_cancel">取消</div>
        </div>
    </div>
</div>

<!-- -----------正在加载------------ -->
<div class="spinner_con a-load" style="display: none;">
    <div>
        <div class="spinner">
            <div class="spinner-container container1">
                <div class="circle1"></div>
                <div class="circle2"></div>
                <div class="circle3"></div>
                <div class="circle4"></div>
            </div>
            <div class="spinner-container container2">
                <div class="circle1"></div>
                <div class="circle2"></div>
                <div class="circle3"></div>
                <div class="circle4"></div>
            </div>
            <div class="spinner-container container3">
                <div class="circle1"></div>
                <div class="circle2"></div>
                <div class="circle3"></div>
                <div class="circle4"></div>
            </div>
        </div>
        <div class="spinner_text">正在加载</div>
    </div>
</div>

<!-- -----------保存成功弹出框------------ -->
<div class="saveinfo s-msg" style="display: none;">
    <div class="sidg">
        <i class="iconfont">&#xe625;</i>
        <div class="sidg_text">保存成功</div>
    </div>
</div>

<!-- ----------纯文本弹出框------------- -->
<div class="clearinfo a-msg" style="display: none;">
    <div class="clear_text">操作失败</div>
</div>
<!-- ----------输入弹出框------------- -->
<div class="clearinfo a-msg" style="display: none;">
    <div class="clear_text">操作失败</div>
</div>
<!-- ----------WiFi扫描结果弹出框------------- -->
<div class="wifi_scan_ret_list">
	<!-- 参数 -->
	<div class="">
		<div class="" style="display:block;">
			<div class="wifi-infro-item-topic"><span class="wifi-infro-ssid">WiFi列表</span></div>
			<ul class="wifi_scan_ret_list-ul"></ul>
		</div>
	</div>
</div>

<!-- -----------wifi密码输入框-------frame----- -->
<div class="en_diak wifi_pwd_input_fram" style="display: none;">
    <div class="enda">
        <div class="endi_title wifi_pwd_input_frame_title">WiFi密码</div>
        <div class="en_mima">
            <input type="text" class="wifi_pwd_input_text" placeholder="请输入密码">
        </div>
        <div class="endia_btn">
            <div class="wifi_pwd_put-btn">确定</div>
            <div class="wifi_pwd_cancel-btn">取消</div>
        </div>
    </div>
</div>

</div>


<!-- -----------输入框背景层-------frame----- -->
<div class="en_dialog" style="display: none;"></div>
</body>
<script type="text/javascript" src="./mqtt_client/jquery-3.2.1.min.js"></script>
<script type="text/javascript" src="./mqtt_client/jquery.nicescroll.js" ></script>
<script type="text/javascript" src="./mqtt_client/index.js"></script>
<script type="text/javascript" src="./mqtt_client/mqttws31.min.js" type="text/javascript"></script>

</html>