$(function(){

    var flag = true;                                                                                                   //判断是否继续获取配置
    var getSafeConfigCount = 0;
    var getWifiConfigCount = 0;
    var getDoorConfigCount = 0;
    var isEdit = false;                                                                                                //判断是否可编辑配置

    init();

    /**
     * 选择配置是否可写
     */
    $("#writer_ch").on('click', function(){
        if ($("#writer_ch").is(":checked")){
            isEdit = true;
            $("input").removeAttr("disabled");
            $('.w-btn, .d-btn').show();
        } else {
            isEdit = false;
            $("input:not(#writer_ch)").attr("disabled","disabled");
            $('.w-btn, .d-btn').hide();
        }
    });

    /**
     * 操作
     */
    $('.o-door').on('click', function(){
        var type = $(this).data('type');
        command(type);
    });

    /**
     * 切换工作模式
     */
    $('.mode').on('click', function(){
        if (!isEdit){
            return false;
        }
        var work_mode = $(this).find('span').data('work_mode');
        $('.mode .r01').removeClass('active');
        $(this).find('span').addClass('active');
        $('.w-mode').val(work_mode);
    });

    /**
     * 保存安全配置
     */
    $('.s-btn').on('click', function(){
        setSafeConfig();
    });

    /**
     * 保存无线配置
     */
    $('.w-btn').on('click', function(){
        setWiFiConfig();
    });

    /**
     * 保存门配置
     */
    $('.d-btn').on('click', function(){
        setDoorConfig();
    });

    /**
     * 配置显示隐藏
     */
    $(".deploy_title").click(function(){
        $(this).siblings(".deploy_con").slideToggle("slow");
        $(this).find(".dep_t_icon").toggleClass("depi_act");
    });

    /**
     * 初始化
     */
    function init(){
        //默认配置不可编辑
        isEdit = false;
        $("input:not(#writer_ch)").attr("disabled","disabled");
        $('.w-btn, .d-btn').hide();
		setTimeout(function () { 
			getSafeConfig();
			getWiFiConfig();
			getDoorConfig();
		}, 500);
        
        setInterval(function(){
            //getStatus();
        }, 2000)
    }

    /**
     * 获取安全配置
     */
    function getSafeConfig(){
        if (!flag){
            return false;
        }
		var postData = {"type":"GetSafeConfig","data":""};
        $.ajax({
			type:'POST',
			url:'/command',
			data:JSON.stringify(postData),
            'success' : function(res){
                if (res.type != 'Reply_GetSafeConfig' || res.error_code != 0){
                    flag = false;
                    error_str = res.error_str || '获取安全配置失败';
                    showMsg(error_str);
                    return false;
                }

                getSafeConfigCount = 0;
                $('.s-token').text(res.data.token);
            },
            'error' : function(){
                if (getSafeConfigCount < 1){
				setTimeout(function () { getSafeConfig();},500);
                    getSafeConfigCount++;
                    return false;
                }
                flag = false;
                showMsg('获取安全配置网络错误');
            }
        });
    }

    /**
     * 获取无线配置
     */
    function getWiFiConfig(){
        if (!flag){
            return false;
        }
		var postData={"type" : "GetWiFiConfig","data" : ""};
        $.ajax({
           type:'POST',
			url:'/command',
			data:JSON.stringify(postData),
            'success' : function(res){
                if (res.type != 'Reply_GetWiFiConfig' || res.error_code != 0){
                    flag = false;
                    error_str = res.error_str || '获取无线配置失败';
                    showMsg(error_str);
                    return false;
                }

                getWifiConfigCount = 0;

                $('.r01').removeClass('active');
                if (res.data.work_mode == 1){
                    $('.i02_r01 span').addClass('active');
                } else {
                    $('.i02_r02 span').addClass('active');
                }
                $('.w-mode').val(res.data.work_mode);

                $('.wa-ssid').val(res.data.wifi_ap_ssid);
                $('.wa-pwd').val(res.data.wifi_ap_pwd);
                $('.ws-ssid').val(res.data.wifi_station_ssid);
                $('.ws-pwd').val(res.data.wifi_station_pwd);
            },
            'error' : function(){
                if (getWifiConfigCount < 1){
                    setTimeout(function () { getWiFiConfig();},500);
                    getWifiConfigCount++;
                    return false;
                }
                flag = false;
                showMsg('获取无线配置网络错误');
            }
        });
    }

    /**
     * 获取门配置
     */
    function getDoorConfig(){
        if (!flag){
            return false;
        }
		var postData={"type" : "GetDoorConfig","data" : ""};
        $.ajax({
           type:'POST',
			url:'/command',
			data:JSON.stringify(postData),
            'success' : function(res){
                if (res.type != 'Reply_GetDoorConfig' || res.error_code != 0){
                    flag = false;
                    error_str = res.error_str || '获取无线配置失败';
                    showMsg(error_str);
                    return false;
                }

                getDoorConfigCount = 0;

                $('.motion_range').val(res.data.motion_range);                                                          //活动范围

                var is_lock_enabled = res.data.is_lock_enabled == 1 ? true : false;                                    //是否启用锁
                $('.is_lock_enabled').prop('checked', is_lock_enabled);
                $('.lock_delay_time').val(res.data.lock_delay_time);                                                    //上锁前等待时间，单位s，取值范围：0-100

                var is_auto_close = res.data.is_auto_close == 1 ? true : false;                                        //是否自动关门
                $('.is_auto_close').prop('checked', is_auto_close);
                $('.open_stay_time').val(res.data.open_stay_time);                                                      //开门停留时间，单位s，取值范围：0-100

                var is_detect_ir = res.data.is_detect_ir == 1 ? true : false;                                          //是否启用感应开门
                $('.is_detect_ir').prop('checked', is_detect_ir);

                var is_double_group = res.data.is_double_group == 1 ? true : false;                                    //是否双门联动
                $('.is_double_group').prop('checked', is_double_group);

                var is_detect_move = res.data.is_detect_move == 1 ? true : false;                                      //是否检测位移开门
                $('.is_detect_move').prop('checked', is_detect_move);
                $('.move_distance').val(res.data.move_distance);                                                        //位移的距离，单位mm，取值范围：0-100

                var is_detect_resist = res.data.is_detect_resist == 1 ? true : false;                                  //是否检测位移开门
                $('.is_detect_resist').prop('checked', is_detect_resist);
                $('.resist_time').val(res.data.resist_time);                                                            //遇阻时间，单位s，取值范围：0.01-1

                $('.open_speed_ratio').val(res.data.open_speed_ratio);                                                  //开门速度比率，相对于最大速度，取值范围：0.0-1.0
                $('.close_speed_ratio').val(res.data.close_speed_ratio);                                                //关门速度比率，相对于最大速度，取值范围：0.0-1.0
                $('.study_speed_ratio').val(res.data.study_speed_ratio);                                                //学习速度比率，相对于最大速度，取值范围：0.0-1.0
                $('.min_speed_ratio').val(res.data.min_speed_ratio);                                                    //最低速度比率，相对于最大速度，取值范围：0.0-1.0
                $('.acc_speed_ratio').val(res.data.acc_speed_ratio);                                                    //加速度速度比率，相对于最大速度，取值范围：0.0-1.0
                $('.dec_speed_ratio').val(res.data.dec_speed_ratio);                                                    //减速度速度比率，相对于最大速度，取值范围：0.0-1.0
                $('.run_low_speed_ratio').val(res.data.run_low_speed_ratio);                                            //最低运行速度比率，相对于最大速度，取值范围：0.0-1.0
                $('.open_pre_dec_speed_ratio').val(res.data.open_pre_dec_speed_ratio);                                  //开门预减速比率，门越重值越大（水平不平会导致开关不一样），取值范围：0.0-1.0
                $('.close_pre_dec_speed_ratio').val(res.data.close_pre_dec_speed_ratio);                                //关门预减速比率，门越重值越大，取值范围：0.0-1.0
            },
            'error' : function(){
                if (getDoorConfigCount < 1){
                    setTimeout(function () { getDoorConfig();},500);
                    getDoorConfigCount++;
                    return false;
                }
                flag = false;
                showMsg('获取门配置网络错误');
            }
        });
    }

    /**
     * 修改安全配置
     */
    function setSafeConfig(){
        //收集参数
        var arr = {};
        arr['type'] = 'SetSafeConfig';
        arr['data'] = {};
        arr['data']['token'] = $('.s-token').val();                                                                 //token

        //验证参数
        if (arr['data']['token'] == ''){
            showMsg('请输入token');
            return false;
        }

        showLoad();
        $.ajax({
            'url' : '/command',
            'type' : 'post',
            'dataType' : 'json',
            'data' : JSON.stringify(arr),
            'success' : function(res){
                hideLoad();
                if (res.error_code == 0){
                    showSuccessMsg('修改成功');
                } else {
                    showMsg('修改失败');
                }
            },
            'error' : function(){
                hideLoad();
                showMsg('修改安全配置失败');
            }
        });
    }

    /**
     * 修改无线配置
     */
    function setWiFiConfig(){
        var arr = {};
        arr['type'] = 'SetWiFiConfig';
        arr['data'] = {};
        arr['data']['work_mode'] = $('.w-mode').val();                                                             //工作模式
        arr['data']['wifi_ap_ssid'] = $('.wa-ssid').val();                                                          //本地模式下路由的名称
        arr['data']['wifi_ap_pwd'] = $('.run_speed_ratio').val();                                               //本地模式下路由的名称
        arr['data']['wifi_station_ssid'] = $('.run_acc_speed_ratio').val();                                       //云端模式下连接的WiFi名称
        arr['data']['wifi_station_pwd'] = $('.study_speed_ratio').val();                                           //云端模式下连接的WiFi密码

        showLoad();
        $.ajax({
            'url' : '/command',
            'type' : 'post',
            'dataType' : 'json',
            'data' : JSON.stringify(arr),
            'success' : function(res){
                hideLoad();
                if (res.error_code == 0){
                    showSuccessMsg('修改成功');
                } else {
                    showMsg('修改失败');
                }
            },
            'error' : function(){
                hideLoad();
                showMsg('修改无线配置失败');
            }
        });
    }

    /**
     * 修改门配置
     */
    function setDoorConfig(){
        //收集参数
        var arr = {};
        arr['type'] = 'SetDoorConfig';
        arr['data'] = {};
        arr['data']['motion_range'] = $('.motion_range').val();                                                         //活动范围，两个止档的距离，取值范围：100-10000
        arr['data']['is_lock_enabled'] = $('.is_lock_enabled').is(':checked') ? "1" : "0";                              //是否启用锁
        arr['data']['lock_delay_time'] = $('.lock_delay_time').val();                                                   //等待时间
        arr['data']['is_auto_close'] = $('.is_auto_close').is(':checked') ? "1" : "0";                                  //是否自动关门
        arr['data']['open_stay_time'] = $('.open_stay_time').val();                                                     //停留时间
        arr['data']['is_detect_ir'] = $('.is_detect_ir').is(':checked') ? "1" : "0";                                    //是否启用感应开门
        arr['data']['is_double_group'] = $('.is_double_group').is(':checked') ? "1" : "0";                              //是否双门联动
        arr['data']['is_detect_move'] = $('.is_detect_move').is(':checked') ? "1" : "0";                                //是否检测位移开门
        arr['data']['move_distance'] = $('.move_distance').val();                                                       //位移的距离，单位mm，取值范围：0-100
        arr['data']['is_detect_resist'] = $('.is_detect_resist').is(':checked') ? "1" : "0";                            //是否检测遇阻
        arr['data']['resist_time'] = $('.resist_time').val();                                                           //遇阻时间，单位s，取值范围：0.01-1
        arr['data']['open_speed_ratio'] = $('.open_speed_ratio').val();                                                 //开门速度比率，相对于最大速度，取值范围：0.0-1.0
        arr['data']['close_speed_ratio'] = $('.close_speed_ratio').val();                                               //关门速度比率，相对于最大速度，取值范围：0.0-1.0
        arr['data']['study_speed_ratio'] = $('.study_speed_ratio').val();                                               //学习速度比率，相对于最大速度，取值范围：0.0-1.0
        arr['data']['min_speed_ratio'] = $('.min_speed_ratio').val();                                                   //最低速度比率，相对于最大速度，取值范围：0.0-1.0
        arr['data']['acc_speed_ratio'] = $('.acc_speed_ratio').val();                                                   //加速度速度比率，相对于最大速度，取值范围：0.0-1.0
        arr['data']['dec_speed_ratio'] = $('.dec_speed_ratio').val();                                                   //减速度速度比率，相对于最大速度，取值范围：0.0-1.0
        arr['data']['run_low_speed_ratio'] = $('.run_low_speed_ratio').val();                                           //最低运行速度比率，相对于期望的速度，过低会导致卡顿，过高会导致速度降不下来，取值范围：0.0-1.0
        arr['data']['open_pre_dec_speed_ratio'] = $('.open_pre_dec_speed_ratio').val();                                 //开门预减速比率，门越重值越大（水平不平会导致开关不一样），取值范围：0.0-1.0
        arr['data']['close_pre_dec_speed_ratio'] = $('.close_pre_dec_speed_ratio').val();                               //关门预减速比率，门越重值越大，取值范围：0.0-1.0


        //验证参数
        if (arr['data']['motion_range'] == ''){
            showMsg('请输入活动范围');
            return false;
        }
        if (arr['data']['motion_range'] < 100 || arr['data']['motion_range'] > 10000){
            showMsg('活动范围在100-10000之间，包括100和10000');
            return false;
        }

        if (arr['data']['lock_delay_time'] == ''){
            showMsg('请输入等待时间');
            return false;
        }
        if (arr['data']['lock_delay_time'] < 0 || arr['data']['lock_delay_time'] > 100){
            showMsg('等待时间范围在0-100之间，包括0和100');
            return false;
        }

        if (arr['data']['open_stay_time'] == ''){
            showMsg('请输入停留时间');
            return false;
        }
        if (arr['data']['open_stay_time'] < 0 || arr['data']['open_stay_time'] > 100){
            showMsg('停留时间范围在0-100之间，包括0和100');
            return false;
        }

        if (arr['data']['move_distance'] == ''){
            showMsg('请输入位移距离');
            return false;
        }
        if (arr['data']['move_distance'] < 0 || arr['data']['move_distance'] > 100){
            showMsg('位移距离范围在0-100之间，包括0和100');
            return false;
        }

        if (arr['data']['resist_time'] == ''){
            showMsg('请输入遇阻时间');
            return false;
        }
        if (arr['data']['resist_time'] < 0.01 || arr['data']['resist_time'] > 1){
            showMsg('遇阻时间范围在0.01-1之间，包括0.01和1');
            return false;
        }
        if (!(/^\d*(?:.\d{0,2})?$/.test(arr['data']['resist_time']))){
            showMsg('遇阻时间保留两位小数');
            return false;
        }

        if (arr['data']['open_speed_ratio'] == ''){
            showMsg('请输入开门速度比率');
            return false;
        }
        if (arr['data']['open_speed_ratio'] < 0.1 || arr['data']['open_speed_ratio'] > 1){
            showMsg('开门速度比率范围在0.1-1之间，包括0.1和1');
            return false;
        }
        if (!(/^\d*(?:.\d{0,2})?$/.test(arr['data']['open_speed_ratio']))){
            showMsg('开门速度比率保留两位小数');
            return false;
        }

        if (arr['data']['close_speed_ratio'] == ''){
            showMsg('请输入关门速度比率');
            return false;
        }
        if (arr['data']['close_speed_ratio'] < 0.1 || arr['data']['close_speed_ratio'] > 1){
            showMsg('关门速度比率范围在0.1-1之间，包括0.1和1');
            return false;
        }
        if (!(/^\d*(?:.\d{0,2})?$/.test(arr['data']['close_speed_ratio']))){
            showMsg('关门速度比率保留两位小数');
            return false;
        }

        if (arr['data']['study_speed_ratio'] == ''){
            showMsg('请输入学习速度比率');
            return false;
        }
        if (arr['data']['study_speed_ratio'] < 0.1 || arr['data']['study_speed_ratio'] > 1){
            showMsg('学习速度比率范围在0.1-1之间，包括0.1和1');
            return false;
        }
        if (!(/^\d*(?:.\d{0,2})?$/.test(arr['data']['study_speed_ratio']))){
            showMsg('学习速度比率保留两位小数');
            return false;
        }

        if (arr['data']['min_speed_ratio'] == ''){
            showMsg('请输入最低速度比率');
            return false;
        }
        if (arr['data']['min_speed_ratio'] < 0.1 || arr['data']['min_speed_ratio'] > 1){
            showMsg('最低速度比率范围在0.1-1之间，包括0.1和1');
            return false;
        }
        if (!(/^\d*(?:.\d{0,2})?$/.test(arr['data']['min_speed_ratio']))){
            showMsg('最低速度比率保留两位小数');
            return false;
        }

        if (arr['data']['acc_speed_ratio'] == ''){
            showMsg('请输入加速度比率');
            return false;
        }
        if (arr['data']['acc_speed_ratio'] < 0.1 || arr['data']['acc_speed_ratio'] > 1){
            showMsg('加速度比率范围在0.1-1之间，包括0.1和1');
            return false;
        }
        if (!(/^\d*(?:.\d{0,2})?$/.test(arr['data']['acc_speed_ratio']))){
            showMsg('加速度比率保留两位小数');
            return false;
        }

        if (arr['data']['dec_speed_ratio'] == ''){
            showMsg('请输入减速度比率');
            return false;
        }
        if (arr['data']['dec_speed_ratio'] < 0.1 || arr['data']['dec_speed_ratio'] > 1){
            showMsg('减速度比率范围在0.1-1之间，包括0.1和1');
            return false;
        }
        if (!(/^\d*(?:.\d{0,2})?$/.test(arr['data']['dec_speed_ratio']))){
            showMsg('减速度比率保留两位小数');
            return false;
        }

        if (arr['data']['run_low_speed_ratio'] == ''){
            showMsg('请输入运行速度比率');
            return false;
        }
        if (arr['data']['run_low_speed_ratio'] < 0.1 || arr['data']['run_low_speed_ratio'] > 1){
            showMsg('运行速度比率范围在0.1-1之间，包括0.1和1');
            return false;
        }
        if (!(/^\d*(?:.\d{0,2})?$/.test(arr['data']['run_low_speed_ratio']))){
            showMsg('运行速度比率保留两位小数');
            return false;
        }

        if (arr['data']['open_pre_dec_speed_ratio'] == ''){
            showMsg('请输入开门预减速度比率');
            return false;
        }
        if (arr['data']['open_pre_dec_speed_ratio'] < 0.1 || arr['data']['open_pre_dec_speed_ratio'] > 1){
            showMsg('开门预减速度比率范围在0.1-1之间，包括0.1和1');
            return false;
        }
        if (!(/^\d*(?:.\d{0,2})?$/.test(arr['data']['open_pre_dec_speed_ratio']))){
            showMsg('开门预减速度比率保留两位小数');
            return false;
        }

        if (arr['data']['close_pre_dec_speed_ratio'] == ''){
            showMsg('请输入关门预减速度比率');
            return false;
        }
        if (arr['data']['close_pre_dec_speed_ratio'] < 0.1 || arr['data']['close_pre_dec_speed_ratio'] > 1){
            showMsg('关门预减速度比率范围在0.1-1之间，包括0.1和1');
            return false;
        }
        if (!(/^\d*(?:.\d{0,2})?$/.test(arr['data']['close_pre_dec_speed_ratio']))){
            showMsg('关门预减速度比率保留两位小数');
            return false;
        }

        showLoad();
        $.ajax({
            'url' : '/command',
            'type' : 'post',
            'dataType' : 'json',
            'data' : JSON.stringify(arr),
            'success' : function(res){
                hideLoad();
                if (res.error_code == 0){
                    showSuccessMsg('修改成功');
                } else {
                    showMsg('修改失败');
                }
            },
            'error' : function(){
                hideLoad();
                showMsg('修改门配置失败');
            }
        });
    }

    /**
     * 一般操作
     */
    function command(type){
        var arr = {};
        arr['type'] = 'Command';
        arr['data'] = {};
        arr['data']['CommandType'] = type;

        showLoad();
        $.ajax({
            'url' : '/command',
            'type' : 'post',
            'dataType' : 'json',
            'data' : JSON.stringify(arr),
            'success' : function(res){
                hideLoad();
                var msg = type == 0 ? '开门' : (type == 1 ? '关门' : (type == 2 ? '锁定' : '解锁'));
                if (res.error_code == 0){
                    msg = msg + '成功';
                    showSuccessMsg(msg);
                } else {
                    msg = msg + '失败';
                    showMsg(msg);
                }
            },
            'error' : function(){
                hideLoad();
                showMsg('网络错误');
            }
        });
    }

    /**
     * 获取运行状态
     */
    function getStatus(){
        var arr = {};
        arr['type'] = 'Heartbeat';
        arr['data'] = {};
        arr['data']['time_tick'] = Date.parse(new Date());

        $.ajax({
            'url' : '/command',
            'type' : 'post',
            'dataType' : 'json',
            'data' : JSON.stringify(arr),
            'success' : function(res){
                if (res.error_code == 0 && res.data.RunState == 0){
                    $('.state').addClass('s-green').removeClass('s-red');
                    $('.state-msg').text('运行正常');
                } else {
                    $('.state').addClass('s-red').removeClass('s-green');
                    $('.state-msg').text('运行异常');
                }
            },
            'error' : function(){
                $('.state').addClass('s-red').removeClass('s-green');
                $('.state-msg').text('运行异常');
                console.log('获取运行状态网络错误');
            }
        });
    }

    /**
     * 显示成功信息
     */
    function showSuccessMsg(text){
        $('.s-msg .sidg_text').text(text);
        $('.s-msg').show();
        setTimeout(function(){
            $('.s-msg').hide();
        }, 1500);
    }

    /**
     * 显示普通文本信息
     */
    function showMsg(text){
        $('.a-msg .clear_text').text(text);
        $('.a-msg').show();
        setTimeout(function(){
            $('.a-msg').hide()
        }, 1500);
    }

    /**
     * 显示加载信息
     */
    function showLoad(){
        $('.a-load').show();
    }

    /**
     * 隐藏加载信息
     */
    function hideLoad(){
        $('.a-load').hide();
    }

});