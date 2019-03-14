$(function(){

    var flag = true;                                                                                                   //判断是否继续获取配置
    var getSafeConfigCount = 0;
    var getWifiConfigCount = 0;
    var getDoorConfigCount = 0;                                                                                         //本地json数据
    var onLineSpeed = {};                                                                                               //速度配置（线上）
    var isEdit = false;                                                                                                //判断是否可编辑门配置
    var isProjectEdit = false;                                                                                         //判断是否可编辑工程参数
    var pwd = 'c92a116ace0e1f1900dc4df0d1e910d1';

    /**
     * 初始化
     */
    init();

    /**
     * 选择门配置是否可写
     */
    $("#writer_ch").on('click', function(){
        if ($("#writer_ch").is(":checked")){
            isEdit = true;
            $(".peiz input").removeAttr("disabled");
            $('.w-btn, .d-btn').show();
        } else {
            isEdit = false;
            $(".peiz input:not(#writer_ch)").attr("disabled","disabled");
            $('.w-btn, .d-btn').hide();
        }
    });

    /**
     * 选择工程配置是否可写
     */
    $("#in_enginer").on('click', function(){
        var status = $(this).data('status');

        if (status == 0){
            //弹出层设置
            var scrollHeight = $(document).scrollTop(),
                dh = $(document).height(),
                windowHeight = $(window).height(),
                popupHeight = $(".en_diak").height(),
                posiTop = (windowHeight - popupHeight)/2 + scrollHeight;

            $(".en_diak").css("top",posiTop);
            $(".en_dialog").css("min-height",dh);
            $(".en_diak,.en_dialog").fadeIn(100);
            $(".p-password").focus();
        } else {
            $('.enginer .slider1').removeClass("intro");
            $('.enginer .slider1 span').removeClass("infot");

            isProjectEdit = false;
            $(".enginer input").attr("disabled", 'disabled');
            $('.p-btn').hide();

            $('#in_enginer').data('status', 0);
        }
    });

    /**
     * 确定密码验证
     */
    $('.en_confirm').on('click', function(){
        var password = $('.p-password').val();

        if (password == ''){
            showMsg('请输入密码');
            return false;
        }

        var res = verify_password(password);
        if (res){
            showSuccessMsg('验证成功');

            $(".en_diak,.en_dialog").fadeOut();
            $(".p-password").val("");
            $('.enginer .slider1').addClass("intro");
            $('.enginer .slider1 span').addClass("infot");

            isProjectEdit = true;
            $(".enginer input").removeAttr("disabled");
            $('.p-btn').show();

            $('#in_enginer').data('status', 1);
        } else {
            showMsg('密码错误，请重新输入');
        }
    });

    /**
     * 取消密码验证
     */
    $('.en_cancel').on('click', function(){
        $('.p-password').val('');
        $(".en_diak,.en_dialog").fadeOut();
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
     * speed方案下拉
     */
    $(".plan_btn").click(function(){
        if (!isEdit){
            return false;
        }
        var status = $(this).data('status');
        if (status == 0){
            $(".plan_ul").slideDown();
        } else {
            $(".plan_ul").slideUp();
        }
        status = status == 1 ? 0 : 1;
        $(this).data('status', status);
    });

    /**
     * 监听speed方案
     */
    $('.speed').on('click', function(){
        var index = $(this).data('index');
        $(this).addClass('active').siblings().removeClass('active');
        $('.plan_btn span').text($(this).text());
        $(".plan_ul").slideUp();
        selectSpeedTemplate(index);
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
     * 保存门配置
     */
    $('.p-btn').on('click', function(){
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
        isProjectEdit = false;
        $("input:not(#writer_ch, .p-password)").attr("disabled","disabled");
        $('.w-btn, .d-btn, .p-btn').hide();

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

        var arr = {
            "type" : "GetSafeConfig",
            "data" : ""
        };

        $.ajax({
            'url' : '/command',
            'type' : 'post',
            'dataType' : 'json',
            'data' : JSON.stringify(arr),
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
                    getSafeConfig();
                    getSafeConfigCount++;
                    return false;
                }
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

        var arr = {
            "type" : "GetWiFiConfig",
            "data" : ""
        };

        $.ajax({
            'url' : '/command',
            'type' : 'post',
            'dataType' : 'json',
            'data' : JSON.stringify(arr),
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
                    getWiFiConfig();
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

        var arr = {
            "type" : "GetDoorConfig",
            "data" : ""
        };

        $.ajax({
            'url' : '/command',
            'type' : 'post',
            'dataType' : 'json',
            'data' : JSON.stringify(arr),
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
                $('.move_speed').val(res.data.move_speed);                                                              //位移的速度，单位mm/s，取值范围：10-1000

                var is_detect_resist = res.data.is_detect_resist == 1 ? true : false;                                  //是否检测位移开门
                $('.is_detect_resist').prop('checked', is_detect_resist);
                $('.resist_time').val(res.data.resist_time);                                                            //遇阻时间，单位s，取值范围：0.01-1

                var is_external_control_enabled = res.data.is_external_control_enabled == 1 ? true : false;            //支持外部接口
                $('.is_external_control_enabled').prop('checked', is_external_control_enabled);

                $('.open_speed').val(res.speed.data.open_speed);                                                              //开门速度，取值范围：100-1000
                $('.close_speed').val(res.speed.data.close_speed);                                                            //关门速度，取值范围：100-1000
                $('.study_speed').val(res.speed.data.study_speed);                                                            //学习速度，取值范围：100-1000
                $('.max_speed').val(res.speed.data.max_speed);                                                                //最大速度，100-1000
                $('.min_speed').val(res.speed.data.min_speed);                                                                //最低速度，100-1000
                $('.acc_speed').val(res.speed.data.acc_speed);                                                                //加速度
                $('.dec_speed').val(res.speed.data.dec_speed);                                                                //减速度

                $('.demand_pre_dec_speed_ratio').val(res.data.speed.demand_pre_dec_speed_ratio);                              //规划预减速比率
                $('.execute_pre_dec_speed_ratio').val(res.data.speed.execute_pre_dec_speed_ratio);                            //执行预减速比率
                $('.start_current').val(res.data.speed.start_current);                                                        //启动电流
                $('.brake_start_current').val(res.data.speed.brake_start_current);                                            //刹车启动电流
                $('.brake_speed_ratio').val(res.data.speed.brake_speed_ratio);                                                //启用刹车的速度差比率
                $('.change_current_1ms').val(res.data.speed.change_current_1ms);                                              //1ms电流变化值
                $('.brake_change_current_1ms').val(res.data.speed.brake_change_current_1ms);                                  //1ms刹车电流变化值

                $('.hall_distance').val(res.data.hall_distance);                                                                //两组hall之间的距离，用来计算滑动轴承的长度，单位mm，取值范围：0-10000
                $('.motor_len').val(res.data.motor_len);                                                                        //电机模组长度，单位mm，取值范围：100-1000
                $('.max_current').val(res.data.max_current);                                                                    //最大电流，单位毫安（占空比满），取值范围：500-10000
                $('.limit_current').val(res.data.limit_current);                                                                //限制电流，单位毫安（运行电流）取值范围：500-10000

                onLineSpeed = saveSpeed(res.data.speed.open_speed, res.data.speed.close_speed, res.data.speed.study_speed, res.data.speed.max_speed, res.data.speed.min_speed, res.data.speed.acc_speed, res.data.speed.dec_speed, res.data.speed.demand_pre_dec_speed_ratio, res.data.speed.execute_pre_dec_speed_ratio, res.data.speed.start_current, res.data.speed.brake_start_current, res.data.speed.brake_speed_ratio, res.data.speed.change_current_1ms, res.data.speed.brake_change_current_1ms);

            },
            'error' : function(){
                if (getDoorConfigCount < 1){
                    getDoorConfig();
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
        arr['data']['work_mode'] = $('.w-mode').val();                                                                  //工作模式
        arr['data']['wifi_ap_ssid'] = $('.wa-ssid').val();                                                              //本地模式下路由的名称
        arr['data']['wifi_ap_pwd'] = $('.run_speed_ratio').val();                                                       //本地模式下路由的名称
        arr['data']['wifi_station_ssid'] = $('.run_acc_speed_ratio').val();                                             //云端模式下连接的WiFi名称
        arr['data']['wifi_station_pwd'] = $('.study_speed_ratio').val();                                                //云端模式下连接的WiFi密码

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
        arr['data']['motion_range'] = parseFloat($('.motion_range').val());                                                     //活动范围，两个止档的距离，取值范围：100-10000
        arr['data']['is_lock_enabled'] = Number($('.is_lock_enabled').is(':checked') ? "1" : "0");                              //是否启用锁
        arr['data']['lock_delay_time'] = parseFloat($('.lock_delay_time').val());                                               //等待时间
        arr['data']['is_auto_close'] = Number($('.is_auto_close').is(':checked') ? "1" : "0");                                  //是否自动关门
        arr['data']['open_stay_time'] = parseFloat($('.open_stay_time').val());                                                 //停留时间
        arr['data']['is_detect_ir'] = Number($('.is_detect_ir').is(':checked') ? "1" : "0");                                    //是否启用感应开门
        arr['data']['is_double_group'] = Number($('.is_double_group').is(':checked') ? "1" : "0");                              //是否双门联动
        arr['data']['is_detect_move'] = Number($('.is_detect_move').is(':checked') ? "1" : "0");                                //是否检测位移开门
        arr['data']['move_distance'] = parseFloat($('.move_distance').val());                                                   //位移的距离，单位mm，取值范围：0-100
        arr['data']['move_speed'] = parseFloat($('.move_speed').val());                                                         //位移的速度，单位mm/s，取值范围：10-1000
        arr['data']['is_detect_resist'] = Number($('.is_detect_resist').is(':checked') ? "1" : "0");                            //是否检测遇阻
        arr['data']['resist_time'] = parseFloat($('.resist_time').val());                                                       //遇阻时间，单位s，取值范围：0.01-1
        arr['data']['is_external_control_enabled'] = parseFloat($('.is_external_control_enabled').is(':checked') ? "1" : "0");  //支持对外接口

        arr['data']['speed'] = {};

        arr['data']['speed']['open_speed'] = parseFloat($('.open_speed').val());                                                 //开门速度，取值范围：100-1000
        arr['data']['speed']['close_speed'] = parseFloat($('.close_speed').val());                                               //关门速度，取值范围：10-1000
        arr['data']['speed']['study_speed'] = parseFloat($('.study_speed').val());                                               //学习速度，取值范围：10-1000
        arr['data']['speed']['max_speed'] = parseFloat($('.max_speed').val());                                                   //最大速度，取值范围：10-1000
        arr['data']['speed']['min_speed'] = parseFloat($('.min_speed').val());                                                   //最低速度，取值范围：10-1000
        arr['data']['speed']['acc_speed'] = parseFloat($('.acc_speed').val());                                                   //加速度
        arr['data']['speed']['dec_speed'] = parseFloat($('.dec_speed').val());                                                   //减速度

        arr['data']['speed']['demand_pre_dec_speed_ratio'] = parseFloat($('.demand_pre_dec_speed_ratio').val());                 //规划预减速比率
        arr['data']['speed']['execute_pre_dec_speed_ratio'] = parseFloat($('.execute_pre_dec_speed_ratio').val());               //执行预减速比率
        arr['data']['speed']['start_current'] = parseFloat($('.start_current').val());                                           //启动电流
        arr['data']['speed']['brake_start_current'] = parseFloat($('.brake_start_current').val());                               //刹车启动电流
        arr['data']['speed']['brake_speed_ratio'] = parseFloat($('.brake_speed_ratio').val());                                   //启用刹车的速度差比率
        arr['data']['speed']['change_current_1ms'] = parseFloat($('.change_current_1ms').val());                                 //1ms电流变化值
        arr['data']['speed']['brake_change_current_1ms'] = parseFloat($('.brake_change_current_1ms').val());                     //1ms刹车电流变化值

        arr['data']['hall_distance'] = parseFloat($('.hall_distance').val());                                                    //两组hall之间的距离，用来计算滑动轴承的长度，单位mm，取值范围：0-10000
        arr['data']['motor_len'] = parseFloat($('.motor_len').val());                                                            //电机模组长度，单位mm，取值范围：100-1000
        arr['data']['max_current'] = parseFloat($('.max_current').val());                                                        //最大电流，单位毫安（占空比满），取值范围：500-10000
        arr['data']['limit_current'] = parseFloat($('.limit_current').val());                                                    //限制电流，单位毫安（运行电流）取值范围：500-10000

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

        if (arr['data']['move_speed'] == ''){
            showMsg('请输入位移速度');
            return false;
        }
        if (arr['data']['move_speed'] < 100 || arr['data']['move_speed'] > 1000){
            showMsg('位移速度范围在100-1000之间，包括100和1000');
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

        if (arr['data']['speed']['open_speed'] == ''){
            showMsg('请输入开门速度');
            return false;
        }
        if (arr['data']['speed']['open_speed'] < 100 || arr['data']['speed']['open_speed'] > 1000){
            showMsg('开门速度范围在100-1000之间，包括100和1000');
            return false;
        }
        if (!(/^\d*(?:.\d{0,2})?$/.test(arr['data']['speed']['open_speed']))){
            showMsg('开门速度保留两位小数');
            return false;
        }

        if (arr['data']['speed']['close_speed'] == ''){
            showMsg('请输入关门速度');
            return false;
        }
        if (arr['data']['speed']['close_speed'] < 10 || arr['data']['speed']['close_speed'] > 1000){
            showMsg('关门速度范围在10-1000之间，包括10和1000');
            return false;
        }
        if (!(/^\d*(?:.\d{0,2})?$/.test(arr['data']['speed']['close_speed']))){
            showMsg('关门速度保留两位小数');
            return false;
        }

        if (arr['data']['speed']['study_speed'] == ''){
            showMsg('请输入学习速度');
            return false;
        }
        if (arr['data']['speed']['study_speed'] < 10 || arr['data']['speed']['study_speed'] > 1000){
            showMsg('学习速度范围在10-1000之间，包括10和1000');
            return false;
        }
        if (!(/^\d*(?:.\d{0,2})?$/.test(arr['data']['speed']['study_speed']))){
            showMsg('学习速度保留两位小数');
            return false;
        }

        if (arr['data']['speed']['max_speed'] == ''){
            showMsg('请输入最大速度');
            return false;
        }
        if (arr['data']['speed']['max_speed'] < 10 || arr['data']['speed']['max_speed'] > 1000){
            showMsg('最大速度范围在10-1000之间，包括10和1000');
            return false;
        }
        if (!(/^\d*(?:.\d{0,2})?$/.test(arr['data']['speed']['max_speed']))){
            showMsg('最大速度保留两位小数');
            return false;
        }

        if (arr['data']['speed']['min_speed'] == ''){
            showMsg('请输入最低速度');
            return false;
        }
        if (arr['data']['speed']['min_speed'] < 10 || arr['data']['speed']['min_speed'] > 1000){
            showMsg('最低速度范围在100-1000之间，包括10和1000');
            return false;
        }
        if (!(/^\d*(?:.\d{0,2})?$/.test(arr['data']['speed']['min_speed']))){
            showMsg('最低速度保留两位小数');
            return false;
        }

        if (arr['data']['speed']['acc_speed'] == ''){
            showMsg('请输入加速度');
            return false;
        }
        if (arr['data']['speed']['acc_speed'] < 10 || arr['data']['speed']['acc_speed'] > 1000){
            showMsg('加速度范围在10-1000之间，包括10和1000');
            return false;
        }
        if (!(/^\d*(?:.\d{0,2})?$/.test(arr['data']['speed']['acc_speed']))){
            showMsg('加速度保留两位小数');
            return false;
        }

        if (arr['data']['speed']['dec_speed'] == ''){
            showMsg('请输入减速度');
            return false;
        }
        if (arr['data']['speed']['dec_speed'] < 10 || arr['data']['speed']['dec_speed'] > 1000){
            showMsg('减速度范围在10-1000之间，包括10和1000');
            return false;
        }
        if (!(/^\d*(?:.\d{0,2})?$/.test(arr['data']['speed']['dec_speed']))){
            showMsg('减速度保留两位小数');
            return false;
        }

        if (arr['data']['speed']['demand_pre_dec_speed_ratio'] == ''){
            showMsg('请输入规划预减速比率');
            return false;
        }
      /*  if (arr['data']['demand_pre_dec_speed_ratio'] < 0.1 || arr['data']['demand_pre_dec_speed_ratio'] > 1.0){
            showMsg('规划预减速比率范围在0.0-1.0之间，包括0.0和1.0');
            return false;
        }*/
        if (!(/^\d*(?:.\d{0,2})?$/.test(arr['data']['speed']['demand_pre_dec_speed_ratio']))){
            showMsg('规划预减速比率保留两位小数');
            return false;
        }

        if (arr['data']['speed']['execute_pre_dec_speed_ratio'] == ''){
            showMsg('请输入执行预减速比率');
            return false;
        }
      /*  if (arr['data']['execute_pre_dec_speed_ratio'] < 0.1 || arr['data']['execute_pre_dec_speed_ratio'] > 1.0){
            showMsg('执行预减速比率范围在0.0-1.0之间，包括0.0和1.0');
            return false;
        }*/
        if (!(/^\d*(?:.\d{0,2})?$/.test(arr['data']['speed']['execute_pre_dec_speed_ratio']))){
            showMsg('执行预减速比率保留两位小数');
            return false;
        }

        if (arr['data']['speed']['start_current'] == ''){
            showMsg('请输入启动电流');
            return false;
        }

        if (arr['data']['speed']['brake_start_current'] == ''){
            showMsg('请输入刹车启动电流');
            return false;
        }

        if (arr['data']['speed']['brake_speed_ratio'] == ''){
            showMsg('请输入启用刹车的速度差比率');
            return false;
        }

        if (arr['data']['speed']['change_current_1ms'] == ''){
            showMsg('请输入1ms电流变化值');
            return false;
        }

        if (arr['data']['speed']['brake_change_current_1ms'] == ''){
            showMsg('请输入1ms刹车电流变化值');
            return false;
        }

        if (arr['data']['hall_distance'] == ''){
            showMsg('请输入两组hall之间的距离');
            return false;
        }
        if (arr['data']['hall_distance'] < 0 || arr['data']['hall_distance'] > 10000){
            showMsg('两组hall之间的距离范围在0-10000之间，包括0和10000');
            return false;
        }
        if (!(/^\d*(?:.\d{0,2})?$/.test(arr['data']['hall_distance']))){
            showMsg('两组hall之间的距离保留两位小数');
            return false;
        }

        if (arr['data']['motor_len'] == ''){
            showMsg('请输入电机模组长度');
            return false;
        }
        if (arr['data']['motor_len'] < 100 || arr['data']['motor_len'] > 1000){
            showMsg('电机模组长度范围在100-1000之间，包括100和1000');
            return false;
        }
        if (!(/^\d*(?:.\d{0,2})?$/.test(arr['data']['motor_len']))){
            showMsg('电机模组长度保留两位小数');
            return false;
        }

        if (arr['data']['max_current'] == ''){
            showMsg('请输入最大电流');
            return false;
        }
        if (arr['data']['max_current'] < 500 || arr['data']['max_current'] > 10000){
            showMsg('最大电流范围在500-10000之间，包括500和10000');
            return false;
        }
        if (!(/^\d*(?:.\d{0,2})?$/.test(arr['data']['max_current']))){
            showMsg('最大电流保留两位小数');
            return false;
        }

        if (arr['data']['limit_current'] == ''){
            showMsg('请输入限制电流');
            return false;
        }
        if (arr['data']['limit_current'] < 500 || arr['data']['limit_current'] > 10000){
            showMsg('限制电流范围在500-10000之间，包括500和10000');
            return false;
        }
        if (!(/^\d*(?:.\d{0,2})?$/.test(arr['data']['limit_current']))){
            showMsg('限制电流保留两位小数');
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
                    onLineSpeed = saveSpeed(arr.open_speed, arr.close_speed, arr.study_speed, arr.max_speed, arr.min_speed, arr.acc_speed, arr.dec_speed, arr.demand_pre_dec_speed_ratio, arr.execute_pre_dec_speed_ratio, arr.start_current, arr.brake_start_current, arr.brake_speed_ratio, arr.change_current_1ms, arr.brake_change_current_1ms);

                    $('.enginer .slider1').removeClass("intro");
                    $('.enginer .slider1 span').removeClass("infot");

                    isProjectEdit = false;
                    $(".enginer input").attr("disabled", 'disabled');
                    $('.p-btn').hide();

                    $('#in_enginer').data('status', 0);
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
        arr['type'] = 'GetStatus';
        arr['data'] = {};

        $.ajax({
            'url' : '/command',
            'type' : 'post',
            'dataType' : 'json',
            'data' : JSON.stringify(arr),
            'success' : function(res){
                reply_status(res.data);
            },
            'error' : function(){
                $('.s-status').text('网络错误');
                console.log('获取运行状态网络错误');
            }
        });
    }

    /**
     * 处理状态
     */
    function reply_status(result){
        if (result.error_code != 0){
            $('.s-status').text(result.error_msg);
            return false;
        }

        //处理参数
        var smstateMsg = '';
        switch (result.data.SMState) {
            case "0":
                smstateMsg = '上电就绪状态';
                break;
            case "1":
                smstateMsg = '上电就绪状态';
                break;
            case "2":
                smstateMsg = '上电就绪状态';
                break;
            case "3":
                smstateMsg = '上电就绪状态';
                break;
            case "4":
                smstateMsg = '上电就绪状态';
                break;
            case "5":
                smstateMsg = '上电就绪状态';
                break;
            case "6":
                smstateMsg = '上电就绪状态';
                break;
            case "7":
                smstateMsg = '上电就绪状态';
                break;
            case "8":
                smstateMsg = '上电就绪状态';
                break;
            case "9":
                smstateMsg = '上电就绪状态';
                break;
            case "10":
                smstateMsg = '上电就绪状态';
                break;
            case "11":
                smstateMsg = '上电就绪状态';
                break;
            case "12":
                smstateMsg = '上电就绪状态';
                break;
            case "13":
                smstateMsg = '上电就绪状态';
                break;
            case "14":
                smstateMsg = '上电就绪状态';
                break;
            case "15":
                smstateMsg = '上电就绪状态';
                break;
            case "16":
                smstateMsg = '上电就绪状态';
                break;
            case "17":
                smstateMsg = '上电就绪状态';
                break;
            case "18":
                smstateMsg = '上电就绪状态';
                break;
            case "19":
                smstateMsg = '上电就绪状态';
                break;
            default:
                break;
        }

        $('.s-status').text(smstateMsg);                                                                                //运行状态
        $('.c-status').text(result.data.CommState);                                                                     //通信状态
        $('.t-degree').text(result.data.Temperature);                                                                   //温度
        $('.w-degree').text(result.data.Wetness);                                                                       //湿度
        $('.p-rate').text(result.data.Power);                                                                           //功率比
        $('.r-time').text(getTime(result.data.RunTime));                                                                //运行时间
    }

    /**
     * 选择使用speed模板
     */
    function selectSpeedTemplate(index){
        var speed = data.speed;
        speed = index == -1 ? onLineSpeed : speed[index];
        useSpeed(speed.open_speed, speed.close_speed, speed.study_speed, speed.max_speed, speed.min_speed, speed.acc_speed, speed.dec_speed, speed.demand_pre_dec_speed_ratio, speed.execute_pre_dec_speed_ratio, speed.start_current, speed.brake_start_current, speed.brake_speed_ratio, speed.change_current_1ms, speed.brake_change_current_1ms)
    }

    /**
     * 保存speed
     */
    function saveSpeed(open_speed, close_speed, study_speed, max_speed, min_speed, acc_speed, dec_speed, demand_pre_dec_speed_ratio, execute_pre_dec_speed_ratio, start_current, brake_start_current, brake_speed_ratio, change_current_1ms, brake_change_current_1ms){
        return {
            "open_speed" : open_speed,
            "close_speed" : close_speed,
            "study_speed" : study_speed,
            "max_speed" : max_speed,
            "min_speed" : min_speed,
            "acc_speed" : acc_speed,
            "dec_speed" : dec_speed,
            "demand_pre_dec_speed_ratio" : demand_pre_dec_speed_ratio,
            "execute_pre_dec_speed_ratio" : execute_pre_dec_speed_ratio,
            "start_current" : start_current,
            "brake_start_current" : brake_start_current,
            "brake_speed_ratio" : brake_speed_ratio,
            "change_current_1ms" : change_current_1ms,
            "brake_change_current_1ms" : brake_change_current_1ms
        };
    }

    /**
     * 使用speed
     */
    function useSpeed(open_speed, close_speed, study_speed, max_speed, min_speed, acc_speed, dec_speed, demand_pre_dec_speed_ratio, execute_pre_dec_speed_ratio, start_current, brake_start_current, brake_speed_ratio, change_current_1ms, brake_change_current_1ms){
        $('.open_speed').val(open_speed);                                                                               //开门速度，取值范围：100-1000
        $('.close_speed').val(close_speed);                                                                             //关门速度，取值范围：100-1000
        $('.study_speed').val(study_speed);                                                                             //学习速度，取值范围：100-1000
        $('.max_speed').val(max_speed);                                                                                 //最大速度，100-1000
        $('.min_speed').val(min_speed);                                                                                 //最低速度，100-1000
        $('.acc_speed').val(acc_speed);                                                                                 //加速度
        $('.dec_speed').val(dec_speed);                                                                                 //减速度

        $('.demand_pre_dec_speed_ratio').val(demand_pre_dec_speed_ratio);                                               //规划预减速比率
        $('.execute_pre_dec_speed_ratio').val(execute_pre_dec_speed_ratio);                                             //执行预减速比率
        $('.start_current').val(start_current);                                                                         //启动电流
        $('.brake_start_current').val(brake_start_current);                                                             //刹车启动电流
        $('.brake_speed_ratio').val(brake_speed_ratio);                                                                 //启用刹车的速度差比率
        $('.change_current_1ms').val(change_current_1ms);                                                               //1ms电流变化值
        $('.brake_change_current_1ms').val(brake_change_current_1ms);                                                   //1ms刹车电流变化值
    }

    /**
     * 密码验证
     */
    function verify_password(password){
        var res = $.md5($.md5(password) + 'o11OiwOI0I0PPJmKrKtJgKg7Yc1U');
        return res == pwd
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

    /**
     * 时间戳转
     */
    function getTime(t){
        h = Math.floor(t / (60 * 60)) > 10 ? Math.floor(t / (60 * 60)) : '0' + Math.floor(t / (60 * 60));
        m = Math.floor((t - h * 60 * 60) / 60) > 10 ? Math.floor((t - h * 60 * 60) / 60) : '0' + Math.floor((t - h * 60 * 60) / 60);
        s = (t - h * 60 * 60) - (m * 60) > 10 ? (t - h * 60 * 60) - (m * 60) : '0' + (t - h * 60 * 60) - (m * 60);
        return h + ':' + m + ':' + s;
    }

});

	
	/**
	 * jQuery MD5 hash algorithm function
	 * 
	 * 	<code>
	 * 		Calculate the md5 hash of a String 
	 * 		String $.md5 ( String str )
	 * 	</code>
	 * 
	 * Calculates the MD5 hash of str using the » RSA Data Security, Inc. MD5 Message-Digest Algorithm, and returns that hash. 
	 * MD5 (Message-Digest algorithm 5) is a widely-used cryptographic hash function with a 128-bit hash value. MD5 has been employed in a wide variety of security applications, and is also commonly used to check the integrity of data. The generated hash is also non-reversable. Data cannot be retrieved from the message digest, the digest uniquely identifies the data.
	 * MD5 was developed by Professor Ronald L. Rivest in 1994. Its 128 bit (16 byte) message digest makes it a faster implementation than SHA-1.
	 * This script is used to process a variable length message into a fixed-length output of 128 bits using the MD5 algorithm. It is fully compatible with UTF-8 encoding. It is very useful when u want to transfer encrypted passwords over the internet. If you plan using UTF-8 encoding in your project don't forget to set the page encoding to UTF-8 (Content-Type meta tag). 
	 * This function orginally get from the WebToolkit and rewrite for using as the jQuery plugin.
	 * 
	 * Example
	 * 	Code
	 * 		<code>
	 * 			$.md5("I'm Persian."); 
	 * 		</code>
	 * 	Result
	 * 		<code>
	 * 			"b8c901d0f02223f9761016cfff9d68df"
	 * 		</code>
	 * 
	 * @alias Muhammad Hussein Fattahizadeh < muhammad [AT] semnanweb [DOT] com >
	 * @link http://www.semnanweb.com/jquery-plugin/md5.html
	 * @see http://www.webtoolkit.info/
	 * @license http://www.gnu.org/licenses/gpl.html [GNU General Public License]
	 * @param {jQuery} {md5:function(string))
	 * @return string
	 */
	
	(function($){
		
		var rotateLeft = function(lValue, iShiftBits) {
			return (lValue << iShiftBits) | (lValue >>> (32 - iShiftBits));
		}
		
		var addUnsigned = function(lX, lY) {
			var lX4, lY4, lX8, lY8, lResult;
			lX8 = (lX & 0x80000000);
			lY8 = (lY & 0x80000000);
			lX4 = (lX & 0x40000000);
			lY4 = (lY & 0x40000000);
			lResult = (lX & 0x3FFFFFFF) + (lY & 0x3FFFFFFF);
			if (lX4 & lY4) return (lResult ^ 0x80000000 ^ lX8 ^ lY8);
			if (lX4 | lY4) {
				if (lResult & 0x40000000) return (lResult ^ 0xC0000000 ^ lX8 ^ lY8);
				else return (lResult ^ 0x40000000 ^ lX8 ^ lY8);
			} else {
				return (lResult ^ lX8 ^ lY8);
			}
		}
		
		var F = function(x, y, z) {
			return (x & y) | ((~ x) & z);
		}
		
		var G = function(x, y, z) {
			return (x & z) | (y & (~ z));
		}
		
		var H = function(x, y, z) {
			return (x ^ y ^ z);
		}
		
		var I = function(x, y, z) {
			return (y ^ (x | (~ z)));
		}
		
		var FF = function(a, b, c, d, x, s, ac) {
			a = addUnsigned(a, addUnsigned(addUnsigned(F(b, c, d), x), ac));
			return addUnsigned(rotateLeft(a, s), b);
		};
		
		var GG = function(a, b, c, d, x, s, ac) {
			a = addUnsigned(a, addUnsigned(addUnsigned(G(b, c, d), x), ac));
			return addUnsigned(rotateLeft(a, s), b);
		};
		
		var HH = function(a, b, c, d, x, s, ac) {
			a = addUnsigned(a, addUnsigned(addUnsigned(H(b, c, d), x), ac));
			return addUnsigned(rotateLeft(a, s), b);
		};
		
		var II = function(a, b, c, d, x, s, ac) {
			a = addUnsigned(a, addUnsigned(addUnsigned(I(b, c, d), x), ac));
			return addUnsigned(rotateLeft(a, s), b);
		};
		
		var convertToWordArray = function(string) {
			var lWordCount;
			var lMessageLength = string.length;
			var lNumberOfWordsTempOne = lMessageLength + 8;
			var lNumberOfWordsTempTwo = (lNumberOfWordsTempOne - (lNumberOfWordsTempOne % 64)) / 64;
			var lNumberOfWords = (lNumberOfWordsTempTwo + 1) * 16;
			var lWordArray = Array(lNumberOfWords - 1);
			var lBytePosition = 0;
			var lByteCount = 0;
			while (lByteCount < lMessageLength) {
				lWordCount = (lByteCount - (lByteCount % 4)) / 4;
				lBytePosition = (lByteCount % 4) * 8;
				lWordArray[lWordCount] = (lWordArray[lWordCount] | (string.charCodeAt(lByteCount) << lBytePosition));
				lByteCount++;
			}
			lWordCount = (lByteCount - (lByteCount % 4)) / 4;
			lBytePosition = (lByteCount % 4) * 8;
			lWordArray[lWordCount] = lWordArray[lWordCount] | (0x80 << lBytePosition);
			lWordArray[lNumberOfWords - 2] = lMessageLength << 3;
			lWordArray[lNumberOfWords - 1] = lMessageLength >>> 29;
			return lWordArray;
		};
		
		var wordToHex = function(lValue) {
			var WordToHexValue = "", WordToHexValueTemp = "", lByte, lCount;
			for (lCount = 0; lCount <= 3; lCount++) {
				lByte = (lValue >>> (lCount * 8)) & 255;
				WordToHexValueTemp = "0" + lByte.toString(16);
				WordToHexValue = WordToHexValue + WordToHexValueTemp.substr(WordToHexValueTemp.length - 2, 2);
			}
			return WordToHexValue;
		};
		
		var uTF8Encode = function(string) {
			string = string.replace(/\x0d\x0a/g, "\x0a");
			var output = "";
			for (var n = 0; n < string.length; n++) {
				var c = string.charCodeAt(n);
				if (c < 128) {
					output += String.fromCharCode(c);
				} else if ((c > 127) && (c < 2048)) {
					output += String.fromCharCode((c >> 6) | 192);
					output += String.fromCharCode((c & 63) | 128);
				} else {
					output += String.fromCharCode((c >> 12) | 224);
					output += String.fromCharCode(((c >> 6) & 63) | 128);
					output += String.fromCharCode((c & 63) | 128);
				}
			}
			return output;
		};
		
		$.extend({
			md5: function(string) {
				var x = Array();
				var k, AA, BB, CC, DD, a, b, c, d;
				var S11=7, S12=12, S13=17, S14=22;
				var S21=5, S22=9 , S23=14, S24=20;
				var S31=4, S32=11, S33=16, S34=23;
				var S41=6, S42=10, S43=15, S44=21;
				string = uTF8Encode(string);
				x = convertToWordArray(string);
				a = 0x67452301; b = 0xEFCDAB89; c = 0x98BADCFE; d = 0x10325476;
				for (k = 0; k < x.length; k += 16) {
					AA = a; BB = b; CC = c; DD = d;
					a = FF(a, b, c, d, x[k+0],  S11, 0xD76AA478);
					d = FF(d, a, b, c, x[k+1],  S12, 0xE8C7B756);
					c = FF(c, d, a, b, x[k+2],  S13, 0x242070DB);
					b = FF(b, c, d, a, x[k+3],  S14, 0xC1BDCEEE);
					a = FF(a, b, c, d, x[k+4],  S11, 0xF57C0FAF);
					d = FF(d, a, b, c, x[k+5],  S12, 0x4787C62A);
					c = FF(c, d, a, b, x[k+6],  S13, 0xA8304613);
					b = FF(b, c, d, a, x[k+7],  S14, 0xFD469501);
					a = FF(a, b, c, d, x[k+8],  S11, 0x698098D8);
					d = FF(d, a, b, c, x[k+9],  S12, 0x8B44F7AF);
					c = FF(c, d, a, b, x[k+10], S13, 0xFFFF5BB1);
					b = FF(b, c, d, a, x[k+11], S14, 0x895CD7BE);
					a = FF(a, b, c, d, x[k+12], S11, 0x6B901122);
					d = FF(d, a, b, c, x[k+13], S12, 0xFD987193);
					c = FF(c, d, a, b, x[k+14], S13, 0xA679438E);
					b = FF(b, c, d, a, x[k+15], S14, 0x49B40821);
					a = GG(a, b, c, d, x[k+1],  S21, 0xF61E2562);
					d = GG(d, a, b, c, x[k+6],  S22, 0xC040B340);
					c = GG(c, d, a, b, x[k+11], S23, 0x265E5A51);
					b = GG(b, c, d, a, x[k+0],  S24, 0xE9B6C7AA);
					a = GG(a, b, c, d, x[k+5],  S21, 0xD62F105D);
					d = GG(d, a, b, c, x[k+10], S22, 0x2441453);
					c = GG(c, d, a, b, x[k+15], S23, 0xD8A1E681);
					b = GG(b, c, d, a, x[k+4],  S24, 0xE7D3FBC8);
					a = GG(a, b, c, d, x[k+9],  S21, 0x21E1CDE6);
					d = GG(d, a, b, c, x[k+14], S22, 0xC33707D6);
					c = GG(c, d, a, b, x[k+3],  S23, 0xF4D50D87);
					b = GG(b, c, d, a, x[k+8],  S24, 0x455A14ED);
					a = GG(a, b, c, d, x[k+13], S21, 0xA9E3E905);
					d = GG(d, a, b, c, x[k+2],  S22, 0xFCEFA3F8);
					c = GG(c, d, a, b, x[k+7],  S23, 0x676F02D9);
					b = GG(b, c, d, a, x[k+12], S24, 0x8D2A4C8A);
					a = HH(a, b, c, d, x[k+5],  S31, 0xFFFA3942);
					d = HH(d, a, b, c, x[k+8],  S32, 0x8771F681);
					c = HH(c, d, a, b, x[k+11], S33, 0x6D9D6122);
					b = HH(b, c, d, a, x[k+14], S34, 0xFDE5380C);
					a = HH(a, b, c, d, x[k+1],  S31, 0xA4BEEA44);
					d = HH(d, a, b, c, x[k+4],  S32, 0x4BDECFA9);
					c = HH(c, d, a, b, x[k+7],  S33, 0xF6BB4B60);
					b = HH(b, c, d, a, x[k+10], S34, 0xBEBFBC70);
					a = HH(a, b, c, d, x[k+13], S31, 0x289B7EC6);
					d = HH(d, a, b, c, x[k+0],  S32, 0xEAA127FA);
					c = HH(c, d, a, b, x[k+3],  S33, 0xD4EF3085);
					b = HH(b, c, d, a, x[k+6],  S34, 0x4881D05);
					a = HH(a, b, c, d, x[k+9],  S31, 0xD9D4D039);
					d = HH(d, a, b, c, x[k+12], S32, 0xE6DB99E5);
					c = HH(c, d, a, b, x[k+15], S33, 0x1FA27CF8);
					b = HH(b, c, d, a, x[k+2],  S34, 0xC4AC5665);
					a = II(a, b, c, d, x[k+0],  S41, 0xF4292244);
					d = II(d, a, b, c, x[k+7],  S42, 0x432AFF97);
					c = II(c, d, a, b, x[k+14], S43, 0xAB9423A7);
					b = II(b, c, d, a, x[k+5],  S44, 0xFC93A039);
					a = II(a, b, c, d, x[k+12], S41, 0x655B59C3);
					d = II(d, a, b, c, x[k+3],  S42, 0x8F0CCC92);
					c = II(c, d, a, b, x[k+10], S43, 0xFFEFF47D);
					b = II(b, c, d, a, x[k+1],  S44, 0x85845DD1);
					a = II(a, b, c, d, x[k+8],  S41, 0x6FA87E4F);
					d = II(d, a, b, c, x[k+15], S42, 0xFE2CE6E0);
					c = II(c, d, a, b, x[k+6],  S43, 0xA3014314);
					b = II(b, c, d, a, x[k+13], S44, 0x4E0811A1);
					a = II(a, b, c, d, x[k+4],  S41, 0xF7537E82);
					d = II(d, a, b, c, x[k+11], S42, 0xBD3AF235);
					c = II(c, d, a, b, x[k+2],  S43, 0x2AD7D2BB);
					b = II(b, c, d, a, x[k+9],  S44, 0xEB86D391);
					a = addUnsigned(a, AA);
					b = addUnsigned(b, BB);
					c = addUnsigned(c, CC);
					d = addUnsigned(d, DD);
				}
				var tempValue = wordToHex(a) + wordToHex(b) + wordToHex(c) + wordToHex(d);
				return tempValue.toLowerCase();
			}
		});
	})(jQuery);
	
	var data = {
  "speed" : [
        {
          "open_speed" : "500.00",
          "close_speed" : "500.00",
          "study_speed" : "100.00",
          "max_speed" : "500.0",
          "min_speed" : "50.00",
          "acc_speed" : "600",
          "dec_speed" : "600",
          "demand_pre_dec_speed_ratio" : "1.20",
          "execute_pre_dec_speed_ratio" : "1.2",

          "start_current" : "60",
          "brake_start_current" : "60",
          "brake_speed_ratio" : "2.50",
          "change_current_1ms" : "1.20",
          "brake_change_current_1ms" : "1.20"
        },
        {
          "open_speed" : "450.00",
          "close_speed" : "450.00",
          "study_speed" : "75.00",
          "max_speed" : "450.00",
          "min_speed" : "50.00",
          "acc_speed" : "600",
          "dec_speed" : "600",
          "demand_pre_dec_speed_ratio" : "4.00",
          "execute_pre_dec_speed_ratio" : "4.00",

          "start_current" : "300",
          "brake_start_current" : "300",
          "brake_speed_ratio" : "2.00",
          "change_current_1ms" : "3.00",
          "brake_change_current_1ms" : "3.00"
        },
        {
          "open_speed" : "400.00",
          "close_speed" : "400.00",
          "study_speed" : "75.00",
          "max_speed" : "450.00",
          "min_speed" : "50.00",
          "acc_speed" : "400",
          "dec_speed" : "400",
          "demand_pre_dec_speed_ratio" : "4.50",
          "execute_pre_dec_speed_ratio" : "4.50",

          "start_current" : "600",
          "brake_start_current" : "800",
          "brake_speed_ratio" : "2.00",
          "change_current_1ms" : "6.00",
          "brake_change_current_1ms" : "6.00"
        },
        {
          "open_speed" : "350.00",
          "close_speed" : "350.00",
          "study_speed" : "75.00",
          "max_speed" : "350.00",
          "min_speed" : "50.00",
          "acc_speed" : "300",
          "dec_speed" : "300",
          "demand_pre_dec_speed_ratio" : "6.00",
          "execute_pre_dec_speed_ratio" : "6.00",

          "start_current" : "800",
          "brake_start_current" : "800",
          "brake_speed_ratio" : "2.00",
          "change_current_1ms" : "6.00",
          "brake_change_current_1ms" : "6.00"
        },
        {
          "open_speed" : "350.00",
          "close_speed" : "350.00",
          "study_speed" : "75.00",
          "max_speed" : "350.00",
          "min_speed" : "50.00",
          "acc_speed" : "240",
          "dec_speed" : "240",
          "demand_pre_dec_speed_ratio" : "9.00",
          "execute_pre_dec_speed_ratio" : "9.00",

          "start_current" : "1000",
          "brake_start_current" : "1000",
          "brake_speed_ratio" : "2.00",
          "change_current_1ms" : "10.00",
          "brake_change_current_1ms" : "10.00"
        },
        {
          "open_speed" : "300.00",
          "close_speed" : "300.00",
          "study_speed" : "75.00",
          "max_speed" : "300.00",
          "min_speed" : "50.00",
          "acc_speed" : "200",
          "dec_speed" : "200",
          "demand_pre_dec_speed_ratio" : "10.00",
          "execute_pre_dec_speed_ratio" : "10.00",

          "start_current" : "1200",
          "brake_start_current" : "1200",
          "brake_speed_ratio" : "2.00",
          "change_current_1ms" : "12.00",
          "brake_change_current_1ms" : "12.00"
        }
    ]
}
