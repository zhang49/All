$(function(){

    var flag = true;                                                                                                   //判断是否继续获取配置

    init();

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
        getSafeConfig();
        getWiFiConfig();
        getDoorConfig();

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

        $.ajax({
            'url' : '/command',
            'type' : 'post',
            'dataType' : 'json',
            'data' : {
                "type" : "GetSafeConfig",
                "data" : ""
            },
            'success' : function(res){
                if (res.type != 'Reply_GetSafeConfig' || res.error_code != 0){
                    flag = false;
                    error_str = res.error_str || '获取安全配置失败';
                    showMsg(error_str);
                    return false;
                }

                $('.s-token').text(res.data.token);
            },
            'error' : function(){
                flag = false;
                showMsg('获取安全配置网络错误');
            }
        });
    }

    /**
     * 获取无效配置
     */
    function getWiFiConfig(){
        if (!flag){
            return false;
        }

        $.ajax({
            'url' : '/command',
            'type' : 'post',
            'dataType' : 'json',
            'data' : {
                "type" : "GetWiFiConfig",
                "data" : ""
            },
            'success' : function(res){
                if (res.type != 'Reply_GetWiFiConfig' || res.error_code != 0){
                    flag = false;
                    error_str = res.error_str || '获取无线配置失败';
                    showMsg(error_str);
                    return false;
                }

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

        $.ajax({
            'url' : '/command',
            'type' : 'post',
            'dataType' : 'json',
            'data' : {
                "type" : "GetDoorConfig",
                "data" : ""
            },
            'success' : function(res){
                if (res.type != 'Reply_GetDoorConfig' || res.error_code != 0){
                    flag = false;
                    error_str = res.error_str || '获取无线配置失败';
                    showMsg(error_str);
                    return false;
                }

                var is_double_group = res.data.is_double_group == 1 ? true : false;
                $('.is_double_group').prop('checked', is_double_group);
                var is_detect_enabled = res.data.is_detect_enabled == 1 ? true : false;
                $('.is_detect_enabled').prop('checked', is_detect_enabled);
                var is_always_open = res.data.is_always_open == 1 ? true : false;
                $('.is_always_open').prop('checked', is_always_open);
                var is_auto_close = res.data.is_auto_close == 1 ? true : false;
                $('.is_auto_close').prop('checked', is_auto_close);
                $('.open_stay_time').val(res.data.open_stay_time);
                var is_lock_enabled = res.data.is_lock_enabled == 1 ? true : false;
                $('.is_lock_enabled').prop('checked', is_lock_enabled);
                $('.lock_delay_time').val(res.data.lock_delay_time);
                $('.run_speed_ratio').val(res.data.run_speed_ratio);
                $('.run_acc_speed_ratio').val(res.data.run_acc_speed_ratio);
                $('.study_speed_ratio').val(res.data.study_speed_ratio);
            },
            'error' : function(){
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
            'data' : arr,
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
            'data' : arr,
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
            arr['data']['is_lock_enabled'] = $('.is_lock_enabled').is(':checked') ? "1" : "0";                              //是否启用锁
            arr['data']['lock_delay_time'] = $('.lock_delay_time').val();                                               //等待时间
            arr['data']['is_auto_close'] = $('.is_auto_close').is(':checked') ? "1" : "0";                                  //是否自动关门
            arr['data']['open_stay_time'] = $('.open_stay_time').val();                                                 //停留时间
            arr['data']['is_detect_enabled'] = $('.is_detect_enabled').is(':checked') ? "1" : "0";                          //是否启用感应开门
            arr['data']['is_always_open'] = $('.is_always_open').is(':checked') ? "1" : "0";                                //是否常开
            arr['data']['is_double_group'] = $('.is_double_group').is(':checked') ? "1" : "0";                             //是否双门联动
            arr['data']['run_speed_ratio'] = $('.run_speed_ratio').val();                                               //运行速度比率，相对于最大速度
            arr['data']['run_acc_speed_ratio'] = $('.run_acc_speed_ratio').val();                                       //运行加速度比率，相对于最大加速度
            arr['data']['study_speed_ratio'] = $('.study_speed_ratio').val();                                           //学习速度比率，相对于最大速度

        //验证参数
        if (arr['data']['lock_delay_time'] == ''){
            showMsg('请输入等待时间');
            return false;
        }
        if (arr['data']['open_stay_time'] == ''){
            showMsg('请输入停留时间');
            return false;
        }
        if (arr['data']['run_speed_ratio'] == ''){
            showMsg('请输入运行速度比率');
            return false;
        }
        if (arr['data']['run_speed_ratio'] < 0.1 || arr['data']['run_speed_ratio'] > 1){
            showMsg('运行速度比率范围在0.1-1之间，包括0.1和1');
            return false;
        }
        if (!(/^\d*(?:.\d{0,2})?$/.test(arr['data']['run_speed_ratio']))){
            showMsg('运行速度比率保留两位小数')
            false;
        }
        if (arr['data']['run_acc_speed_ratio'] == ''){
            showMsg('请输入运行加速度比率');
            return false;
        }
        if (arr['data']['run_acc_speed_ratio'] < 0.1 || arr['data']['run_acc_speed_ratio'] > 1){
            showMsg('运行加速度比率范围在0.1-1之间，包括0.1和1');
            return false;
        }
        if (!(/^\d*(?:.\d{0,2})?$/.test(arr['data']['run_acc_speed_ratio']))){
            showMsg('运行加速度比率保留两位小数')
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
            showMsg('学习速度比率保留两位小数')
            return false;
        }

        showLoad();
        $.ajax({
            'url' : '/command',
            'type' : 'post',
            'dataType' : 'json',
            'data' : arr,
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
            'data' : arr,
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
            'data' : arr,
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
        }, 1000);
    }

    /**
     * 显示普通文本信息
     */
    function showMsg(text){
        $('.a-msg .clear_text').text(text);
        $('.a-msg').show();
        setTimeout(function(){
            $('.a-msg').hide()
        }, 1000);
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