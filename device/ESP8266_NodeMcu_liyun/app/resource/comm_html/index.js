$(function(){
    var flag = true;       			   //判断是否继续获取配置
    var getSafeConfigCount = 0;
    var getWifiConfigCount = 0;
    var getNormalConfigCount = 0;      //本地json数据
    var onLineSpeed = {};      		   //速度配置（线上）
    var isEdit = false;      		   //判断是否可编辑门配置
    var isProjectEdit = false;         //判断是否可编辑工程参数
    var pwd = '123456789';
	var choice_ssid = "";
	var lightDutyIsWriting=0;
	var lightDutyTouchValue=0;
	
	var rayIsWriting=0;
	var rayTouchValue=0;
	var replyGetStatusCount=0;
	if(typeof RS == "undefined"){
		var RS = {
			Normal : 0,
			Waiting : 1,
			Unknow : 2,
			Error : 3
		}
	}
	var runStatus=RS.Waiting;
	var lastscrollHeight;
    /**
     * 初始化
     */
    init();

    /**
     * 选择配置是否可写
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
     * 选择部分配置是否可写，需要密码验证
     */
    $("#in_enginer").on('click', function(){
        var status = $(this).data('status');
        if (status == 0){
			var windowHeight = $(window).height();
			var windowWidth = $(window).width();
			var popupHeight = $(".wifi_scan_ret_list").height();
			var posiTop = (windowHeight - popupHeight)/2;
			lastscrollHeight = $(document).scrollTop();
			$(".en_diak.config_frame").css({'top':posiTop+lastscrollHeight});
			
			$('.en_diak.config_frame,.en_dialog').fadeIn(100);
			ShowShandowBg();
			var bwidth = $("body").width();
			document.body.style.overflow='hidden';
			document.addEventListener('touchmove',bodyScroll,false);
			bwidth = windowWidth-$(window).width();
			if(bwidth!=0)$("body").css({'marginLeft':bwidth});
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

            $(".en_diak.config_frame,.en_dialog").fadeOut();
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
		$('.en_dialog').click();
    });

    /**
     * 操作
     */
    $('.op-control-closed').on('click', function(){
        var type = $(this).data('type');
        control(type,"open");
    });
    $('.op-control-opened').on('click', function(){
        var type = $(this).data('type');
        control(type,"close");
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
     * 扫描wifi
     */
    $('.wifiscan-btn').on('click', function(){
        startScanfWiFitest();
    });
	
    /**
     * wifi连接密码输入，确认
     */
    $('.wifi_pwd_put-btn').on('click', function(){
        var password = $('.wifi_pwd_input_text').val();
        if (password == ''){
			
        }else if(password.length<8){
            showMsg('请输入密码(至少八位)');
			return false;
		}
		setWiFiConfig(1,choice_ssid,password);
		document.body.removeEventListener('touchmove',bodyScroll,false);   
		$('.en_dialog').click();
    });

    /**
     * wifi密码输入，取消
     */
    $('.wifi_pwd_cancel-btn').on('click', function(){
        $('.wifi_pwd_input_text').val('');
		$(".en_diak.wifi_pwd_input_fram").fadeOut(50);
		$('.wifi_scan_ret_list').fadeIn(100);
    });
	function ShowShandowBg(){
		$(".en_dialog")[0].style.height =  document.body.scrollHeight+"px";
    	$('.en_dialog').fadeIn(100);
	}
	/**
	 * 点击弹窗外的区域(遮蔽层)
	 */
    $('.en_dialog').on('click', function(){
		document.body.removeEventListener('touchmove',bodyScroll,false);
		$("body").css({"position":"initial","height":"auto",'marginLeft':0});
		document.body.style.overflow='';
		document.documentElement.scrollTop = lastscrollHeight;
		$('.en_diak,.wifi_scan_ret_list,.en_dialog').fadeOut(50);
    });
	 
		
    /**
     * 保存配置
     */
    $('.d-btn').on('click', function(){
        setDoorConfig();
    });

    /**
     * 保存配置
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
		//初始化滑动列表
		$(".wifi_scan_ret_list").niceScroll({cursorcolor:"#cccccc",cursoropacitymax: 0});
		
		//获取配置
		setTimeout(
			function(){
				GetConnectSynData()
				getWiFiConfig();
				//getSafeConfig();
				//getNormalConfigCount();
		},500);
        //获取状态（心跳包）
        setInterval(function(){
			switch(runStatus){
				case RS.Normal:
					$('.conn-indicator').css('background','#00ff00');
				break;
				case RS.Waiting:
					$('.conn-indicator').css('background','#0000ff');
				break;
				case RS.Unknow:
					$('.conn-indicator').css('background','#000000');
				break;
				case RS.Error:
					$('.conn-indicator').css('background','#ff0000');
				break;
			}
			replyGetStatusCount++;
			if(replyGetStatusCount>=4){
				runStatus=RS.Error;
			}else if(replyGetStatusCount>=2){
				runStatus=RS.Waiting;
			}
            getStatus();
        }, 2000)
    }

	
	/**
     * 获取同步配置
     */
	 function GetConnectSynData(){
		if (!flag){
			return false;
		}
		var arr = {
			"type" : "GetConnectSynData",
			"data" : ""
		};

        $.ajax({
            'url' : '/command',
            'type' : 'post',
            'dataType' : 'json',
            'data' : JSON.stringify(arr),
            'success' : function(res){
				Reply_GetConnectSynData(res);
            },
            'error' : function(){
                //showMsg('获取配置网络错误');
            }
        });
		
	}
	
	function Reply_GetConnectSynData(res){
		getSafeConfigCount = 0;
		var $lightinputRange = $('#lightDutyValue');
		var $lightinputRange = $('#lightDutyValue');
		$lightinputRange.val(res.data.duty).change();
		
		var $rayinputRange = $('#rayAlarmValue');
		var $rayinputRange = $('#rayAlarmValue');
		$rayinputRange.val(res.data.alarm['ray-value']).change();
		console.log("+++"+res.data.alarm['ray-value']);
        $('.s-token').text(res.data.mac);
		
		var control_type = res.data.control_type;
		var index = res.data.index;
		var op = res.data.op;
		
		var i;
		var relays=[];
		relays=res.data.relays;
		for(i=0;i<relays.length;i++){
			var temp = [];
			temp.data=[];
			temp.data.device_type = relays[i].device_type;
			temp.data.index = relays[i].index;
			temp.data.op = relays[i].op;
			temp.error_code=0;
			console.log(temp);
			Reply_Control(temp,1);
		}
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
                $('.s-token').text(res.data.mac);
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
     * 获取WiFi配置
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
     * 修改WiFi配置
     */
    function setWiFiConfig(flag,sta_ssid,sta_pwd){
        var arr = {};
		arr['type'] = 'SetWiFiConfig';
        arr['data'] = {};
		if(flag==1){
			arr['data']['wifi_station_ssid'] = sta_ssid;
			arr['data']['wifi_station_pwd'] = sta_pwd;
		}else{
			arr['data']['wifi_ap_ssid'] = $('.wa-ssid').val();                                                              //本地模式下路由的名称
			arr['data']['wifi_ap_pwd'] = $('.wa-pwd').val();                                                       //本地模式下路由的名称
			arr['data']['wifi_station_ssid'] = $('.ws-ssid').val();                                             //云端模式下连接的WiFi名称
			arr['data']['wifi_station_pwd'] = $('.ws-pwd').val();                                                //云端模式下连接的WiFi密码
		}
        arr['data']['work_mode'] = $('.w-mode').val();                                                                  //工作模式
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
     * 扫描WiFi测试
     */
	function startScanfWiFitest(){
    	var jsonstr="{	\"type\":	\"Reply_GetWiFiScan\",	\"data\":	{		\"ap_count\":	24,		\"ap\":	[{				\"ssid\":	\"CMCC-WEB\",				\"rssi\":	170,				\"enc\":	0,				\"channel\":	1			}, {				\"ssid\":	\"CMCC-WEB\",				\"rssi\":	167,				\"enc\":	0,				\"channel\":	1			}, {				\"ssid\":	\"v10\",				\"rssi\":	161,				\"enc\":	3,				\"channel\":	1			}, {				\"ssid\":	\"GCUWIFI\",				\"rssi\":	172,				\"enc\":	0,				\"channel\":	1			}, {				\"ssid\":	\"CMCC-WEB\",				\"rssi\":	181,				\"enc\":	0,				\"channel\":	1			}, {				\"ssid\":	\"Ares\",				\"rssi\":	209,				\"enc\":	3,				\"channel\":	6			}, {				\"ssid\":	\"ChinaNet\",				\"rssi\":	199,				\"enc\":	0,				\"channel\":	4			}, {				\"ssid\":	\"GCUWIFI\",				\"rssi\":	188,				\"enc\":	0,				\"channel\":	6			}, {				\"ssid\":	\"CMCC-WEB\",				\"rssi\":	188,				\"enc\":	0,				\"channel\":	6			}, {				\"ssid\":	\"CMCC-WEB\",				\"rssi\":	176,				\"enc\":	0,				\"channel\":	6			}]	}}"
    	var res=JSON.parse(jsonstr);
    	if (res.type != 'Reply_GetWiFiScan'){
    		flag = false;
    		error_str = res.error_str || '获取无线配置失败';
    		showMsg(error_str);
    		return false;
    	}
    	$(".wifi_scan_ret_list-ul").empty();
    	for(i=0;i<res.data.ap.length;i++){
    	  $(".wifi_scan_ret_list-ul").append("<li class=\"wifi-infro-item\"><i class=\"iconfont\">&#xe673;</i>&nbsp&nbsp<span class=\"wifi-infro-ssid\">"+res.data.ap[i].ssid+"</span></li>");	  
    	}
    	$(".wifi-infro-item").click(function(){
    		//find查找所有的子元素，会一直查找，跨层级查找 
    		var choice_ssid= $(this).find(".wifi-infro-ssid").html();
    		console.log("wifi-infro-item choose_name : "+choice_ssid);
			var windowHeight = $(window).height();
			var popupHeight = $(".en_diak.wifi_pwd_input_fram").height();
			var posiTop = (windowHeight - popupHeight)/2;
			lastscrollHeight = $(document).scrollTop();
    		$('.wifi_scan_ret_list').fadeOut();
			$(".en_diak.wifi_pwd_input_fram").css({'top':posiTop+lastscrollHeight});
			$(".en_diak.wifi_pwd_input_fram").fadeIn(100);
			$(".endi_title.wifi_pwd_input_frame_title").html(choice_ssid+"密码");
            $(".wifi_pwd_input_text").focus();
    	});
		
		var windowHeight = $(window).height();
		var windowWidth = $(window).width();
		var popupHeight = $(".wifi_scan_ret_list").height();
		var posiTop = (windowHeight - popupHeight)/2;
		lastscrollHeight = $(document).scrollTop();
		$(".wifi_scan_ret_list").css({'top':posiTop+lastscrollHeight});
		$(".wifi_scan_ret_list").niceScroll({railoffset:true});
    	$('.wifi_scan_ret_list').fadeIn(100);
    	ShowShandowBg();
		document.body.style.overflow='hidden';
    	document.addEventListener('touchmove',bodyScroll,false);
		$("body").css({'marginLeft':windowWidth-$(window).width()});

    }

    function bodyScroll(event){  
        event.preventDefault();  
    } 

	
	/**
	 *扫描wifi
	 */
	 function startScanfWiFi(){
        showLoad();
        var arr = {
            "type" : "GetWiFiScan",
            "data" : ""
        };
        $.ajax({
            'url' : '/command',
            'type' : 'post',
            'dataType' : 'json',
            'data' : JSON.stringify(arr),
            'success' : function(res){
				Reply_GetWiFiScan(res);
				},
            'error' : function(){
				hideLoad();
                showMsg('扫描失败');
            }
        });
    }
	
	function Reply_GetWiFiScan(res){
		hideLoad();
		if (res.type != 'Reply_GetWiFiScan' || res.error_code != 0){
			flag = false;
			error_str = res.error_str || '获取无线列表失败';
			showMsg(error_str);
			return false;
		}

		$(".wifi_scan_ret_list-ul").empty();
    	for(i=0;i<res.data.ap.length;i++){
    	  $(".wifi_scan_ret_list-ul").append("<li class=\"wifi-infro-item\"><i class=\"iconfont\">&#xe673;</i>&nbsp&nbsp<span class=\"wifi-infro-ssid\">"+res.data.ap[i].ssid+"</span></li>");	  
    	}
    	$(".wifi-infro-item").click(function(){
    		//find查找所有的子元素，会一直查找，跨层级查找 
    		var choice_ssid= $(this).find(".wifi-infro-ssid").html();
    		console.log("wifi-infro-item choose_name : "+choice_ssid);
			var windowHeight = $(window).height();
			var popupHeight = $(".en_diak.wifi_pwd_input_fram").height();
			var posiTop = (windowHeight - popupHeight)/2;
			lastscrollHeight = $(document).scrollTop();
    		$('.wifi_scan_ret_list').fadeOut();
			$(".en_diak.wifi_pwd_input_fram").css({'top':posiTop+lastscrollHeight});
			$(".en_diak.wifi_pwd_input_fram").fadeIn(100);
			$(".endi_title.wifi_pwd_input_frame_title").html(choice_ssid+"密码");
            $(".wifi_pwd_input_text").focus();
    	});
		
		var windowHeight = $(window).height();
		var windowWidth = $(window).width();
		var popupHeight = $(".wifi_scan_ret_list").height();
		var posiTop = (windowHeight - popupHeight)/2;
		lastscrollHeight = $(document).scrollTop();
		$(".wifi_scan_ret_list").css({'top':posiTop+lastscrollHeight});
		$(".wifi_scan_ret_list").niceScroll({railoffset:true});
    	$('.wifi_scan_ret_list').fadeIn(100);
    	ShowShandowBg();
		document.body.style.overflow='hidden';
    	document.addEventListener('touchmove',bodyScroll,false);
		$("body").css({'marginLeft':windowWidth-$(window).width()});
	}
	
    /**
     * 一般操作
     */
    function control(index,op){
        showLoad();
        var arr = {};
        arr['type'] = 'Control';
        arr['data'] = {};
		//0 - relay
        arr['data']['device_type'] = 0;
        arr['data']['index'] = index;
		if(op=="open"){
			arr['data']['op'] = 1;
		}
		else if(op=="close"){
			arr['data']['op'] = 0;
		}
        $.ajax({
            'url' : '/command',
            'type' : 'post',
            'dataType' : 'json',
            'data' : JSON.stringify(arr),
            'success' : function(res){
				Reply_Control(res);
			},
            'error' : function(){
                hideLoad();
                showMsg('网络错误');
            }
        });
		
    }
	
	function Reply_Control(res,noShowMsg){
		hideLoad();
		var control_type = res.data.control_type;
		var index = res.data.index;
		var op = res.data.op;
		var msg = index == 0 ? '1' : (index == 1 ? '2' : (index == 2 ? '3' : '4'));
		if (res.error_code == 0){
			if(op==1){
				$(".op-control-closed[data-type=" + index + "]").unbind("click");
				$(".op-control-closed[data-type=" + index + "]").toggleClass("op-control-opened");
				$(".op-control-opened[data-type=" + index + "]").removeClass("op-control-closed");
				$(".op-control-opened[data-type=" + index + "]").delegate($(this),'click', function(){
					var index = $(this).data('type');
					control(index,"close");
				});
			}
			else if(op==0){
				$(".op-control-opened[data-type=" + index + "]").unbind("click");
				$(".op-control-opened[data-type=" + index + "]").toggleClass("op-control-closed");
				
				$(".op-control-closed[data-type=" + index + "]").removeClass("op-control-opened");
				$(".op-control-closed[data-type=" + index + "]").delegate($(this),'click', function(){
					var index = $(this).data('type');
					control(index,"open");
				});
			}
			msg = msg + '成功';
			if(!noShowMsg)showSuccessMsg(msg);
		} else {
			msg = msg + '失败';
			if(!noShowMsg)showMsg(msg);
		}
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
                Reply_GetStatus(res);
            },
            'error' : function(){
                $('.s-status').text('网络错误');
                console.log('获取运行状态网络错误');
            }
        });
    }
	function Reply_GetStatus(res){
		if (res.error_code != 0){
			$('.s-status').text(res.error_str);
			runStatus=RS.Unknow;
			return false;
		}
		replyGetStatusCount--;
		if(runStatus!=RS.Normal){
			runStatus=RS.Normal;
			replyGetStatusCount=0;
		}
		
		data=res.data;
        //处理参数
        var smstateMsg = '';
        switch (data.sm_state) {
            case 0:
                smstateMsg = '运行中';
				break;
			case 1:
			smstateMsg = '设备异常';
                break;
            default:
                break;
        }
        $('.s-status').text(smstateMsg);                         //运行状态
        $('.c-status').text(data.comm_state);                   //通信状态
		
        $('.t-degree').text(data.temperatur/100+" ℃");  //温度
        $('.w-degree').text(data.humidity+" %");                         //湿度
        //$('.p-rate').text(data.power);                           //功率比
		$('.r-degree').text(data['ray-value']);
        $('.r-time').text(getTime(data.run_time));            //运行时间
		
    }
   
   /**
	 *设置光强报警值
	 */
	function SetRayAlarmValue(value){
		var arr = {};
		arr['type'] = 'SetRayAlarmValue';
        arr['data'] = {};
		arr['data']['ray-value'] = value-0;
		$.ajax({
			'url' : '/command',
			'type' : 'post',
			'dataType' : 'json',
			'data' : JSON.stringify(arr),
			'success' : function(res){
				return Reply_SetRayAlarmValue(res);
			},
			'error' : function(){
                showMsg('网络错误');
				return -1;
			}
		});
		
	}
	function Reply_SetRayAlarmValue(res){
		var $rayinputRange = $('#rayAlarmValue');
		var $rayinputRange = $('#rayAlarmValue');
		$rayinputRange.val(res.data['ray-value']).change();
		return res.data['ray-value'];
	}
	
	/**
	 *设置灯Duty
	 */
	function SetLigthDuty (value){
		var arr = {};
		arr['type'] = 'SetLigthDuty';
        arr['data'] = {};
		arr['data']['duty'] = value-0;
        $.ajax({
            'url' : '/command',
            'type' : 'post',
            'dataType' : 'json',
            'data' : JSON.stringify(arr),
            'success' : function(res){
				return Reply_SetLigthDuty(res);
            },
            'error' : function(){
                showMsg('网络错误');
				return -1;
            }
        });
	}
	function Reply_SetLigthDuty(res){
		var $lightinputRange = $('#lightDutyValue');
		var $lightinputRange = $('#lightDutyValue');
		$lightinputRange.val(res.data.duty).change();
		return res.data.duty;
	}
	/**
	 *获取灯Duty
	 */
	function getLigthDuty(value){
		var arr = {};
		arr['type'] = 'GetLigthDuty';
        arr['data'] = {};
		arr['data']['duty'] = value;
        $.ajax({
            'url' : '/command',
            'type' : 'post',
            'dataType' : 'json',
            'data' : JSON.stringify(arr),
            'success' : function(res){
				res.data.duty
            },
            'error' : function(){
                showMsg('设置失败');
            }
        });
	}
    /**
     * 密码验证
     */
    function verify_password(password){
        var res = $.md5($.md5(password) + '123456789');
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
	/**
	 *滑动条
	 */
    $(function() {
        var $document   = $(document);
        var selector    = '[data-rangeslider]';
        var $inputRange = $(selector);

        // Example functionality to demonstrate a value feedback
        // and change the output's value.
        function valueOutput(element) {
            var value = element.value;
            var output = element.parentNode.getElementsByTagName('output')[0];
            return value;
        }
        // Update value output
        $document.on('input', selector, function(e) {
			if($(this).attr('id')=="lightDutyValue"){
				var value=valueOutput(e.target);
				lightDutyTouchValue=value;
				if(lightDutyIsWriting==0){			
					lightDutyIsWriting=1;
					var $inputRange = $('#lightDutyValue', e.target.parentNode);					
					var ret=SetLigthDuty(lightDutyTouchValue);
					if(ret==-1){
						//failed							
					}else{
						//set duty text							
					}
					setTimeout(function(){
						$inputRange.val(lightDutyTouchValue).change();
						console.log(lightDutyTouchValue);
						lightDutyIsWriting=0}
					,300);
				}
			}else if($(this).attr('id')=="rayAlarmValue"){
				var value=valueOutput(e.target);
				rayTouchValue=value;
				if(rayIsWriting==0){			
					rayIsWriting=1;
					var $inputRange = $('#rayAlarmValue', e.target.parentNode);					
					var ret=SetRayAlarmValue(rayTouchValue);
					if(ret==-1){
						//failed							
					}else{
						//set text							
					}
					setTimeout(function(){
						$inputRange.val(rayTouchValue).change();
						console.log(rayTouchValue);
						rayIsWriting=0}
					,300);
				}
			}
            
        });

        // Initialize the elements
        $inputRange.rangeslider({
            polyfill: false
        });


    });



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

/*! rangeslider.js - v1.2.1 | (c) 2015 @andreruffert | MIT license | https://github.com/andreruffert/rangeslider.js */
!function(a){"use strict";"function"==typeof define&&define.amd?define(["jquery"],a):a("object"==typeof exports?require("jquery"):jQuery)}(function(a){"use strict";function b(){var a=document.createElement("input");return a.setAttribute("type","range"),"text"!==a.type}function c(a,b){var c=Array.prototype.slice.call(arguments,2);return setTimeout(function(){return a.apply(null,c)},b)}function d(a,b){return b=b||100,function(){if(!a.debouncing){var c=Array.prototype.slice.apply(arguments);a.lastReturnVal=a.apply(window,c),a.debouncing=!0}return clearTimeout(a.debounceTimeout),a.debounceTimeout=setTimeout(function(){a.debouncing=!1},b),a.lastReturnVal}}function e(a){return a&&(0===a.offsetWidth||0===a.offsetHeight||a.open===!1)}function f(a){for(var b=[],c=a.parentNode;e(c);)b.push(c),c=c.parentNode;return b}function g(a,b){function c(a){"undefined"!=typeof a.open&&(a.open=a.open?!1:!0)}var d=f(a),e=d.length,g=[],h=a[b];if(e){for(var i=0;e>i;i++)g[i]=d[i].style.cssText,d[i].style.display="block",d[i].style.height="0",d[i].style.overflow="hidden",d[i].style.visibility="hidden",c(d[i]);h=a[b];for(var j=0;e>j;j++)d[j].style.cssText=g[j],c(d[j])}return h}function h(b,e){if(this.$window=a(window),this.$document=a(document),this.$element=a(b),this.options=a.extend({},l,e),this.polyfill=this.options.polyfill,this.onInit=this.options.onInit,this.onSlide=this.options.onSlide,this.onSlideEnd=this.options.onSlideEnd,this.polyfill&&k)return!1;this.identifier="js-"+i+"-"+j++,this.startEvent=this.options.startEvent.join("."+this.identifier+" ")+"."+this.identifier,this.moveEvent=this.options.moveEvent.join("."+this.identifier+" ")+"."+this.identifier,this.endEvent=this.options.endEvent.join("."+this.identifier+" ")+"."+this.identifier,this.toFixed=(this.step+"").replace(".","").length-1,this.$fill=a('<div class="'+this.options.fillClass+'" />'),this.$handle=a('<div class="'+this.options.handleClass+'" />'),this.$range=a('<div class="'+this.options.rangeClass+'" id="'+this.identifier+'" />').insertAfter(this.$element).prepend(this.$fill,this.$handle),this.$element.css({position:"absolute",width:"1px",height:"1px",overflow:"hidden",opacity:"0"}),this.handleDown=a.proxy(this.handleDown,this),this.handleMove=a.proxy(this.handleMove,this),this.handleEnd=a.proxy(this.handleEnd,this),this.init();var f=this;this.$window.on("resize."+this.identifier,d(function(){c(function(){f.update()},300)},20)),this.$document.on(this.startEvent,"#"+this.identifier+":not(."+this.options.disabledClass+")",this.handleDown),this.$element.on("change."+this.identifier,function(a,b){if(!b||b.origin!==f.identifier){var c=a.target.value,d=f.getPositionFromValue(c);f.setPosition(d)}})}var i="rangeslider",j=0,k=b(),l={polyfill:!0,rangeClass:"rangeslider",disabledClass:"rangeslider--disabled",fillClass:"rangeslider__fill",handleClass:"rangeslider__handle",startEvent:["mousedown","touchstart","pointerdown"],moveEvent:["mousemove","touchmove","pointermove"],endEvent:["mouseup","touchend","pointerup"]};h.prototype.init=function(){this.update(!0),this.$element[0].value=this.value,this.onInit&&"function"==typeof this.onInit&&this.onInit()},h.prototype.update=function(a){a=a||!1,a&&(this.min=parseFloat(this.$element[0].getAttribute("min")||0),this.max=parseFloat(this.$element[0].getAttribute("max")||100),this.value=parseFloat(this.$element[0].value||this.min+(this.max-this.min)/2),this.step=parseFloat(this.$element[0].getAttribute("step")||1)),this.handleWidth=g(this.$handle[0],"offsetWidth"),this.rangeWidth=g(this.$range[0],"offsetWidth"),this.maxHandleX=this.rangeWidth-this.handleWidth,this.grabX=this.handleWidth/2,this.position=this.getPositionFromValue(this.value),this.$element[0].disabled?this.$range.addClass(this.options.disabledClass):this.$range.removeClass(this.options.disabledClass),this.setPosition(this.position)},h.prototype.handleDown=function(a){if(a.preventDefault(),this.$document.on(this.moveEvent,this.handleMove),this.$document.on(this.endEvent,this.handleEnd),!((" "+a.target.className+" ").replace(/[\n\t]/g," ").indexOf(this.options.handleClass)>-1)){var b=this.getRelativePosition(a),c=this.$range[0].getBoundingClientRect().left,d=this.getPositionFromNode(this.$handle[0])-c;this.setPosition(b-this.grabX),b>=d&&b<d+this.handleWidth&&(this.grabX=b-d)}},h.prototype.handleMove=function(a){a.preventDefault();var b=this.getRelativePosition(a);this.setPosition(b-this.grabX)},h.prototype.handleEnd=function(a){a.preventDefault(),this.$document.off(this.moveEvent,this.handleMove),this.$document.off(this.endEvent,this.handleEnd),this.$element.trigger("change",{origin:this.identifier}),this.onSlideEnd&&"function"==typeof this.onSlideEnd&&this.onSlideEnd(this.position,this.value)},h.prototype.cap=function(a,b,c){return b>a?b:a>c?c:a},h.prototype.setPosition=function(a){var b,c;b=this.getValueFromPosition(this.cap(a,0,this.maxHandleX)),c=this.getPositionFromValue(b),this.$fill[0].style.width=c+this.grabX+"px",this.$handle[0].style.left=c+"px",this.setValue(b),this.position=c,this.value=b,this.onSlide&&"function"==typeof this.onSlide&&this.onSlide(c,b)},h.prototype.getPositionFromNode=function(a){for(var b=0;null!==a;)b+=a.offsetLeft,a=a.offsetParent;return b},h.prototype.getRelativePosition=function(a){var b=this.$range[0].getBoundingClientRect().left,c=0;return"undefined"!=typeof a.pageX?c=a.pageX:"undefined"!=typeof a.originalEvent.clientX?c=a.originalEvent.clientX:a.originalEvent.touches&&a.originalEvent.touches[0]&&"undefined"!=typeof a.originalEvent.touches[0].clientX?c=a.originalEvent.touches[0].clientX:a.currentPoint&&"undefined"!=typeof a.currentPoint.x&&(c=a.currentPoint.x),c-b},h.prototype.getPositionFromValue=function(a){var b,c;return b=(a-this.min)/(this.max-this.min),c=b*this.maxHandleX},h.prototype.getValueFromPosition=function(a){var b,c;return b=a/(this.maxHandleX||1),c=this.step*Math.round(b*(this.max-this.min)/this.step)+this.min,Number(c.toFixed(this.toFixed))},h.prototype.setValue=function(a){a!==this.value&&this.$element.val(a).trigger("input",{origin:this.identifier})},h.prototype.destroy=function(){this.$document.off("."+this.identifier),this.$window.off("."+this.identifier),this.$element.off("."+this.identifier).removeAttr("style").removeData("plugin_"+i),this.$range&&this.$range.length&&this.$range[0].parentNode.removeChild(this.$range[0])},a.fn[i]=function(b){var c=Array.prototype.slice.call(arguments,1);return this.each(function(){var d=a(this),e=d.data("plugin_"+i);e||d.data("plugin_"+i,e=new h(this,b)),"string"==typeof b&&e[b].apply(e,c)})}});

