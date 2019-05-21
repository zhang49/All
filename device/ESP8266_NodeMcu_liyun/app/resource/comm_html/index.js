
jQuery(document).ready(function($) {
	// 这行是 Opera 的补丁, 少了它 Opera 是直接用跳的而且画面闪烁 by willin
    $body = (window.opera) ? (document.compatMode == "CSS1Compat" ? $('html') : $('body')) : $('html,body');
});
//设置ajax头，laravel post需要此设置
$.ajaxSetup({
	headers: {
		'X-CSRF-TOKEN': $('meta[name="csrf-token"]').attr('content')
	}
});
$(function(){
    var flag = true;       			   //判断是否继续获取配置
    var getNormalConfigCount = 0;      //本地json数据
    var onLineSpeed = {};      		   //速度配置（线上）
    var isEdit = false;      		   //判断配置是否可编辑
    var isProjectEdit = false;         //判断是否可编辑工程参数
    var pwd = '123456789';
	var choice_ssid = "";
	var lightDutyIsWriting=0;
	var lightDutyTouchValue=0;
	var getStatusFlag = true;
	
	var rayIsWriting=0;
	var rayTouchValue=0;
	var getStatusErrorCount=0;
	var operatorLock = false;
	var operatorWaitTime = 800;//ms
	var initOver=false;
	var hasSynConnectData=false;
	
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
     * 初始化
     */
    function init(){
	
		//初始化滑动列表
		$(".wifi_scan_ret_list").niceScroll({cursorcolor:"#cccccc",cursoropacitymax: 0});
		

		if(!getStatusFlag)return;
		setTimeout(
			function(){
				GetConnectSynData();
		},500);
		var initInterval = setInterval(function(){
		if(hasSynConnectData){
			//锁定配置
			$("#writer_ch").click();			
			initOver=true;
			clearInterval(initInterval);
			return;
		}},300);

		
		var getStatusTimer=setInterval(function(){
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
					clearInterval(getStatusTimer);
				break;
			}
			 getStatusErrorCount++;
			 if(getStatusErrorCount>=3){
			 	runStatus=RS.Error;
			 	showMsg('无法与设备建立通信');
			 }else if(getStatusErrorCount>=2){
			 	runStatus=RS.Waiting;
			 }
             getStatus();
        }, 3000);
    }
	
	 
	/**
     * 获取同步配置
     */
	 function GetConnectSynData(){
		if (!flag){
			return false;
		}
		var reqType='GetConnectSynData';
		var arr={};
		arr.type=reqType;
		arr.data='';
		$.ajax({
             url: "/command",
             type: "POST",
				dataType:'json',
             data:JSON.stringify(arr),
            success: function(recv){
				hasSynConnectData=true;
				Reply_GetConnectSynData(recv);
            },
			error:function(){
				msg = '操作失败';
				showMsg(msg);
			}
        });
	}

	function Reply_GetConnectSynData(res){
		var $lightAlarmInputRange = $('#rayAlarmValue');
		var LightLuxAlarmValue=res.data.LightLuxAlarmValue;

		var MACAddress=res.data.MACAddress;
		var StationSSID=res.data.StationSSID;
		var ApSSID=res.data.ApSSID;


		$lightAlarmInputRange.val(LightLuxAlarmValue).change();
        $('.s-token').text(MACAddress);
		$('.wa-ssid').val(ApSSID);
		//?? this？
		$('#con-s-ssid').text(StationSSID);
		//$('.wa-pwd').val();
		//$('.ws-ssid').val(StationSSID);
		//$('.ws-pwd').val();

	}


	/**
	 * 获取运行状态
	 */
	function getStatus(){
		var reqType='GetStatus';
		var arr={};
		arr.type=reqType;
		arr.data='123';
		$.ajax({
			url: "/command",
			type: "POST",
			dataType:'json',
			data:JSON.stringify(arr),
			success: function(res){
				Reply_GetStatus(res);

			},
			error:function(){
				msg = '操作失败';
				showMsg(msg);
			}
		});
	}

	function Reply_GetStatus(res){
		if (res.error_code != 0){
			$('.s-status').text(res.error_str);
			runStatus=RS.Unknow;
			return false;
		}
		var $LightLuxInputRange = $('#lightDutyValue');
		$LightLuxInputRange.val(res.data.LightLuminance).change();
		$('.t-degree').text(res.data.CurrentTemperature/100 + " ℃");
		//$('.t-degree').text(res.data.SoilMoisture/100 + " ℃");
		$('.w-degree').text(res.data.CurrentHumidity/100 + " %");
		$('.s-status').text(res.data.zsta); 	   //运行状态
		$('.r-time').text(getTime(res.data.Runtime));  	//运行时间
		$('.r-degree').text(res.data.LightLux+'%');
		var Relay=res.data.Relay;
		var temp={};
		temp.data={};
		temp.data.Relay=Relay;
		Reply_Control(temp,1);

		var val;
		var d = new Date();
		var nowTime=d.getTime();
		
		
		getStatusErrorCount--;

		//$('.c-status').text(res.comm_state); 	 //通信状态
		//$('.p-rate').text(res.power);    	    //功率比
	}

    /**
     * 获取WiFi配置
     */
    function getWiFiConfig(){
        if (!flag){
            return false;
        }
		var arr={};
		arr.type='GetWiFiInfo';
		arr.data='';
		$.ajax({
			url: "/command",
			type: "POST",
			dataType:'json',
			data:JSON.stringify(arr),
			success: function(res){
				Reply_GetWiFiConfig(res);
			},
			error:function(){
				msg = '操作失败';
				showMsg(msg);
			}
		});
    }
	
	function Reply_GetWiFiConfig(res){
		if (res.error_code != 0){
			flag = false;
			error_str = res.error_str || '获取无线配置失败';
			showMsg(error_str);
			return false;
		}
		$('.wa-ssid').val(res.data.wifi_ap_ssid);
		$('.wa-pwd').val(res.data.wifi_ap_pwd);
		$('.ws-ssid').val(res.data.wifi_station_ssid);
		$('.ws-pwd').val(res.data.wifi_station_pwd);
        $('.s-token').text(res.data.mac);
	}
	
    /**
     * 修改WiFi配置
     */
    function setWiFiApConfig(flag,sta_ssid,sta_pwd){
        var arr = {};
		arr.type = 'SetWiFiApConfig';
        arr.data = {};
		arr.data.wifi_ap_ssid= $('.wa-ssid').val();   //模块自身路由名称
		arr.data.wifi_ap_pwd = $('.wa-pwd').val();     //密码
		$.ajax({
			url: "/command",
			type: "POST",
			dataType:'json',
			data:JSON.stringify(arr),
			success: function(res){
				Reply_setWiFiApConfig(res);
			},
			error:function(){
				msg = '操作失败';
				showMsg(msg);
			}
		});
		showLoad();
       
    }
	function Reply_setWiFiApConfig(res){
		hideLoad();
		if (res.error_code == 0){
			showSuccessMsg('修改成功');
		} else {
			showMsg('修改失败');
		}
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
		var reqType='GetWiFiScan';
		var arr={};
		arr.type=reqType;
		arr.data='';
		$.ajax({
             url: "/command",
             type: "POST",
				dataType:'json',
             data:JSON.stringify(arr),
            success: function(msg){
				
              Reply_GetWiFiScan(msg);
            },
			error:function(){
				msg = '操作失败';
				showMsg(msg);
				
				
				
				/*
				
                flag = true;

                var html = '<div class="n-fount">找不到网络:(</div><div class="dewf_s a-wifi">重新扫描</div>';
                $('.w-list').html(html);

                $('.w-list').show();
                $('.w-progress').hide();
				
				*/
			}
		});
	 }
	 
	
	function Reply_GetWiFiScan(res){
		hideLoad();
		if (res.error_code != 0){
			showMsg(res.error_str);
		}

		var html = '';
		if (res.data.ApCount > 0){
			var ap = res.data.ApInfo;
			html += '<div class="dewf dewf_title"><div class="dewf_name">网络</div><div class="dewf_status">加密</div><div class="dewf_sign">强度</div></div>';

			for (var i=0;i<ap.length;i++){
				//处理网络是否开放
				var enc = '';
				switch(ap[i].enc){
					case 0:
						enc = "OPEN";
						break;
					case 1:
					case 2:
					case 3:
					case 4:
					case 5:
						enc = "CLOSED";
						break;
					default:
						enc = "CLOSED";
						break;
				}

				//处理网络信息强度
				var signal_pct = parseInt(100*ap[i].rssi/256.0) + '%';

				html += '<div class="dewf s-wifi" data-ssid="'+ ap[i].ssid +'" data-enc="'+ ap[i].enc +'"><div class="dewf_name">'+ ap[i].ssid +'</div><div class="dewf_status">'+ enc +'</div><div class="dewf_sign">'+ signal_pct +'</div></div>';
			}
		} else {
			html = '<div class="n-fount">找不到网络:(</div>';
		}
		//html += '<div class="dewf_s a-wifi">重新扫描</div>';

		$('.w-list').html(html);

		$('.w-list').show();
		$('.w-progress').hide();

		flag = true;
	
		
		return;
		
		hideLoad();
		if (res.error_code != 0){
			flag = false;
			error_str = res.error_str || '获取无线列表失败';
			showMsg(error_str);
			return false;
		}

		$(".wifi_scan_ret_list-ul").empty();
    	for(i=0;i<res.data.ApInfo.length;i++){
    	  $(".wifi_scan_ret_list-ul").append("<li class=\"wifi-infro-item\"><i class=\"iconfont\">&#xe673;</i>&nbsp&nbsp<span class=\"wifi-infro-ssid\">"+res.data.ApInfo[i].ssid+"</span></li>");	  
    	}
		var windowHeight = $(window).height();
		var windowWidth = $(window).width();
		var popupHeight = $(".wifi_scan_ret_list").height();
		var posiTop = (windowHeight - popupHeight)/2;
		lastscrollHeight = $(document).scrollTop();
		$(".wifi_scan_ret_list").css({'top':posiTop+lastscrollHeight});
		$(".wifi_scan_ret_list").niceScroll({railoffset:true});
    	$('.wifi_scan_ret_list').fadeIn(100);
        $('.wifi_pwd_input_text').val('');
    	ShowShandowBg();
		document.body.style.overflow='hidden';
    	document.addEventListener('touchmove',bodyScroll,false);
		$("body").css({'marginLeft':windowWidth-$(window).width()});
	}
	
	

	/**
	 *连接wifi
	 */
	function WiFiConnect(ssid,pwd){
		if (!flag){
			return false;
		}
		showLoad();
		var arr = {};
		arr.type = "WiFiConnect";
		arr.data = {};
		arr.data.wifi_station_ssid = ssid;
		arr.data.wifi_station_pwd = pwd;
		$.ajax({
			url: "/command",
			type: "POST",
			dataType:'json',
			data:JSON.stringify(arr),
			success: function(recv){
				hideLoad();
				Reply_WiFiConnect(recv);
			},
			error:function(){
				msg = '操作失败';
				showMsg(msg);
			}
		});
	}
	
	function Reply_WiFiConnect(res){
		hideLoad();
		if (res.error_code == 0){
			showMsg('WiFi连接成功，请重新刷新页面');
		}
		else{
            showMsg('WiFi连接失败');
		}
	}
	
    /**
     * 继电器控制
     */
     function control(index,op){
		showLoad();
		var reqType='Control';
        var arr = {};
        arr.type = reqType;
        arr.data = {};
        arr.data['index'] = index;
		if(op=="open"){
			arr.data['status'] = 1;
		}
		else if(op=="close"){
			arr.data['status'] = 0;
		}
		$.ajax({
             url: "/command",
             type: "POST",
				dataType:'json',
             data:JSON.stringify(arr),
            success: function(res){
				Reply_Control(res);
            },
			error:function(){
				msg = '操作失败';
				showMsg(msg);
			}
        });
		
    }
	
	/**
     * 操作继电器 1 - 常开 0 - 常闭
     */
	function Reply_Control(res,noShowMsg){
		if(!noShowMsg)hideLoad();
		if(typeof res.error_code != "undefined" && res.error_code!=0){
			msg = '操作失败';
			if(!noShowMsg)showMsg(msg);
			return;
		}

		// var msg = index == 0 ? '1' : (index == 1 ? '2' : (index == 2 ? '3' : '4'));
		for (var i=0;i<res.data.Relay.length;i++) {
			//now is open
			if(res.data.Relay[i]==1){
				$(".op-control-closed[data-type=" + i + "]").unbind("click");
				$(".op-control-closed[data-type=" + i + "]").toggleClass("op-control-opened");
				$(".op-control-opened[data-type=" + i + "]").removeClass("op-control-closed");
				$(".op-control-opened[data-type=" + i + "]").delegate($(this),'click', function(){
					var index = $(this).data('type');
					control_close_operator(index);
				});
			}else if(res.data.Relay[i]==0) {
				$(".op-control-opened[data-type=" + i + "]").unbind("click");
				$(".op-control-opened[data-type=" + i + "]").toggleClass("op-control-closed");
				$(".op-control-closed[data-type=" + i + "]").removeClass("op-control-opened");
				$(".op-control-closed[data-type=" + i + "]").delegate($(this), 'click', function () {
					var index = $(this).data('type');
					control_open_operator(index);
				});
			}
		}
		if(!noShowMsg)showSuccessMsg('操作成功');
	}
	
	 /**
     * 操作步进电机
     */
	function motorOperator(type){
		showLoad();
        var arr = {};
		var reqType="RayMotorStop";
		switch(type){
			case 0:
				reqType="RayMotorCW";
				break;
			case 1:
				reqType="RayMotorStop";
				break;
			case 2:
				reqType="RayMotorCCW";
				break;
		}
		var arr={};
		arr.type=reqType;
		arr.data='';
		$.ajax({
             url: "/command",
             type: "POST",
				dataType:'json',
             data:JSON.stringify(arr),
            success: function(res){
				Reply_RayMotor(res);
            },
			error:function(){
				msg = '操作失败';
				showMsg(msg);
			}
        });
	}
	
	function Reply_RayMotor(res){
		hideLoad();
		var msg="";
		if(res.error_code==0){
			msg = msg + '操作成功';
			showSuccessMsg(msg);
		} else {
			msg = '操作失败';
			showMsg(msg);
		}
	}

	/**
	 *设置光强预警值
	 */
	function SetLightLuxAlarmValue(value){		
		var reqType='SetLightLuxAlarmValue';
		var arr={};
		arr.type=reqType;
		arr.data={};
		arr.data['LightLuxAlarmValue']=parseInt(value);
		$.ajax({
             url: "/command",
             type: "POST",
				dataType:'json',
             data:JSON.stringify(arr),
            success: function(res){
				//Reply_SetLightLuxAlarmValue(res);
            },
			error:function(){
				msg = '操作失败';
				showMsg(msg);
			}
        });
	}
	function Reply_SetLightLuxAlarmValue(res){
		var $rayinputRange = $('#rayAlarmValue');
		$rayinputRange.val(res.data.LightLuxAlarmValue).change();
		return res.data.LightLuxAlarmValue;
	}
	
	/**
	 *设置灯亮度
	 */
	function SetLightLuminance(value){
		var reqType='SetLightLuminance';
		var arr={};
		arr.type=reqType;
		arr.data={};
		arr.data['LightLuminance']=parseInt(value);
		$.ajax({
             url: "/command",
             type: "POST",
				dataType:'json',
             data:JSON.stringify(arr),
            success: function(res){
				//Reply_SetLightLuminance(res);
				
            },
			error:function(){
				msg = '操作失败';
				showMsg(msg);
			}
        });
        
	}
	function Reply_SetLightLuminance(res){
		var $lightinputRange = $('#lightDutyValue');
		$lightinputRange.val(res.data.LightLuminance).change();
		return res.data.LightLuminance;
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
	 * 背景遮蔽层
	 */
	function ShowShandowBg(){
		$(".en_dialog")[0].style.height =  document.body.scrollHeight+"px";
		$('.en_dialog').fadeIn(100);
	}

	/**
	 * 选择配置是否可写
	 */
	$("#writer_ch").on('click', function(){
		if ($("#writer_ch").is(":checked")){
			isEdit = true;
			$(".peiz input").removeAttr("disabled");
			$('.w-btn, .wifiscan-btn, .d-btn').show();
		} else {
			isEdit = false;
			$(".peiz input:not(#writer_ch)").attr("disabled","disabled");
			$('.w-btn, .wifiscan-btn, .d-btn').hide();

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
		//handle  index
	});

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

	///////////////////点击触发ajax事件
	/**
	 * 点击Relay操作
	 */
	$('.op-control-closed').on('click', function(){
		var index = $(this).data('type');
		control_open_operator(index);
	});
	$('.op-control-opened').on('click', function(){
		var index = $(this).data('type');
		control_close_operator(index);
	});

	function control_open_operator(index){
		if(runStatus==RS.Error){

			showMsg('无法连接设备');
			return;
		}
		if(operatorLock){
			console.log("to fast.");
			return;
		}
		operatorLock=true;
		control(index,"open");
		setTimeout(function(){
			operatorLock=false;
		},operatorWaitTime);
	};
	function control_close_operator(index){
		if(runStatus==RS.Error){
			showMsg('无法连接设备');
			return;
		}
		if(operatorLock){
			console.log("to fast.");
			return;
		}
		operatorLock=true;
		control(index,"close");
		setTimeout(function(){
			operatorLock=false;
		},operatorWaitTime);
	};

	/**
	 * 点击Motor操作
	 */
	$('.op-motor').on('click', function(){
		if(runStatus==RS.Error){
			showMsg('无法连接设备');
			return;
		}
		if(operatorLock){
			console.log("to fast.");
			return;
		}
		operatorLock=true;
		var type = $(this).data('type');
		motorOperator(type);
		setTimeout(function(){
			operatorLock=false;
		},operatorWaitTime);
	});

	/**
	 * 保存无线配置
	 */
	$('.w-btn').on('click', function(){
		if(runStatus==RS.Error){
			showMsg('无法连接设备');
			return;
		}
		setWiFiApConfig();
	});

	/**
	 * 扫描wifi
	 */
	$('.wifiscan-btn').on('click', function(){
		if(runStatus==RS.Error){
			showMsg('无法连接设备');
			return;
		}
		$('.w-progress').show();
		startScanfWiFi();
	});

	$('.a-wifi').on('click',function () {
		alert('dds');
		if(runStatus==RS.Error){
			showMsg('无法连接设备');
			return;
		}
		$('.w-progress').show();
		startScanfWiFi();
	});
	/**
	 * 选择要连接的WiFi ssid（旧列表弹出框）
	 */
	$(".wifi-infro-item").click(function(){
		choice_ssid= $(this).find(".wifi-infro-ssid").html();
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

	/**
	 * 选择要连接的WiFi ssid
	 */
	$(document).on('click', '.s-wifi', function(){
		if(runStatus==RS.Error){
			showMsg('无法连接设备');
			return;
		}
		ShowShandowBg();
		document.body.style.overflow='hidden';
		document.addEventListener('touchmove',bodyScroll,false);
		$("body").css({'marginLeft':windowWidth-$(window).width()});

		var windowWidth = $(window).width();
		choice_ssid = $(this).data('ssid');
		console.log("wifi-infro-item choose_name : "+choice_ssid);
		var windowHeight = $(window).height();
		var popupHeight = $(".en_diak.wifi_pwd_input_fram").height();
		var posiTop = (windowHeight - popupHeight)/2;
		lastscrollHeight = $(document).scrollTop();
		$('.wifi_scan_ret_list').fadeOut();
		$(".en_diak.wifi_pwd_input_fram").css({'top':posiTop+lastscrollHeight});
		$(".en_diak.wifi_pwd_input_fram").fadeIn(100);
		$(".endi_title.wifi_pwd_input_frame_title").html(choice_ssid+"密码");

		return;
	});

	/**
	 * wifi连接密码输入，确认选项
	 */
	$('.wifi_pwd_put-btn').on('click', function(){
		if(runStatus==RS.Error){
			showMsg('无法连接设备');
			return;
		}
		var password = $('.wifi_pwd_input_text').val();
		if (password == ''){

		}else if(password.length<8){
			showMsg('请输入密码(至少八位)');
			return false;
		}
		WiFiConnect(choice_ssid,password);
		document.body.removeEventListener('touchmove',bodyScroll,false);
		$('.en_dialog').click();
	});

	/**
	 * wifi密码输入，取消选项
	 */
	$('.wifi_pwd_cancel-btn').on('click', function(){
		$(".en_diak.wifi_pwd_input_fram").fadeOut(50);
		$('.en_dialog').click();
	});

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
			if(!initOver)return;
			if($(this).attr('id')=="lightDutyValue"){
				var value=valueOutput(e.target);
				lightDutyTouchValue=value;
				if(lightDutyIsWriting==0){			
					lightDutyIsWriting=1;
					var $inputRange = $('#lightDutyValue', e.target.parentNode);					
					var ret=SetLightLuminance(lightDutyTouchValue);
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
					var ret=SetLightLuxAlarmValue(rayTouchValue);
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
	
	/**
	 *格式化时间，便于调试
	 */ 
	Date.prototype.Format = function (fmt) { //author: meizz 
		var o = {
			"M+": this.getMonth() + 1, //月份 
			"d+": this.getDate(), //日 
			"h+": this.getHours(), //小时 
			"m+": this.getMinutes(), //分 
			"s+": this.getSeconds(), //秒 
			"q+": Math.floor((this.getMonth() + 3) / 3), //季度 
			"S": this.getMilliseconds() //毫秒 
		};
		if (/(y+)/.test(fmt)) fmt = fmt.replace(RegExp.$1, (this.getFullYear() + "").substr(4 - RegExp.$1.length));
		for (var k in o)
			if (new RegExp("(" + k + ")").test(fmt)) fmt = fmt.replace(RegExp.$1, (RegExp.$1.length == 1) ? (o[
				k]) : (("00" + o[k]).substr(("" + o[k]).length)));
		return fmt;
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

/*! rangeslider.js - v1.2.1 | (c) 2015 @andreruffert | MIT license | https://github.com/andreruffert/rangeslider.js */
!function(a){"use strict";"function"==typeof define&&define.amd?define(["jquery"],a):a("object"==typeof exports?require("jquery"):jQuery)}(function(a){"use strict";function b(){var a=document.createElement("input");return a.setAttribute("type","range"),"text"!==a.type}function c(a,b){var c=Array.prototype.slice.call(arguments,2);return setTimeout(function(){return a.apply(null,c)},b)}function d(a,b){return b=b||100,function(){if(!a.debouncing){var c=Array.prototype.slice.apply(arguments);a.lastReturnVal=a.apply(window,c),a.debouncing=!0}return clearTimeout(a.debounceTimeout),a.debounceTimeout=setTimeout(function(){a.debouncing=!1},b),a.lastReturnVal}}function e(a){return a&&(0===a.offsetWidth||0===a.offsetHeight||a.open===!1)}function f(a){for(var b=[],c=a.parentNode;e(c);)b.push(c),c=c.parentNode;return b}function g(a,b){function c(a){"undefined"!=typeof a.open&&(a.open=a.open?!1:!0)}var d=f(a),e=d.length,g=[],h=a[b];if(e){for(var i=0;e>i;i++)g[i]=d[i].style.cssText,d[i].style.display="block",d[i].style.height="0",d[i].style.overflow="hidden",d[i].style.visibility="hidden",c(d[i]);h=a[b];for(var j=0;e>j;j++)d[j].style.cssText=g[j],c(d[j])}return h}function h(b,e){if(this.$window=a(window),this.$document=a(document),this.$element=a(b),this.options=a.extend({},l,e),this.polyfill=this.options.polyfill,this.onInit=this.options.onInit,this.onSlide=this.options.onSlide,this.onSlideEnd=this.options.onSlideEnd,this.polyfill&&k)return!1;this.identifier="js-"+i+"-"+j++,this.startEvent=this.options.startEvent.join("."+this.identifier+" ")+"."+this.identifier,this.moveEvent=this.options.moveEvent.join("."+this.identifier+" ")+"."+this.identifier,this.endEvent=this.options.endEvent.join("."+this.identifier+" ")+"."+this.identifier,this.toFixed=(this.step+"").replace(".","").length-1,this.$fill=a('<div class="'+this.options.fillClass+'" />'),this.$handle=a('<div class="'+this.options.handleClass+'" />'),this.$range=a('<div class="'+this.options.rangeClass+'" id="'+this.identifier+'" />').insertAfter(this.$element).prepend(this.$fill,this.$handle),this.$element.css({position:"absolute",width:"1px",height:"1px",overflow:"hidden",opacity:"0"}),this.handleDown=a.proxy(this.handleDown,this),this.handleMove=a.proxy(this.handleMove,this),this.handleEnd=a.proxy(this.handleEnd,this),this.init();var f=this;this.$window.on("resize."+this.identifier,d(function(){c(function(){f.update()},300)},20)),this.$document.on(this.startEvent,"#"+this.identifier+":not(."+this.options.disabledClass+")",this.handleDown),this.$element.on("change."+this.identifier,function(a,b){if(!b||b.origin!==f.identifier){var c=a.target.value,d=f.getPositionFromValue(c);f.setPosition(d)}})}var i="rangeslider",j=0,k=b(),l={polyfill:!0,rangeClass:"rangeslider",disabledClass:"rangeslider--disabled",fillClass:"rangeslider__fill",handleClass:"rangeslider__handle",startEvent:["mousedown","touchstart","pointerdown"],moveEvent:["mousemove","touchmove","pointermove"],endEvent:["mouseup","touchend","pointerup"]};h.prototype.init=function(){this.update(!0),this.$element[0].value=this.value,this.onInit&&"function"==typeof this.onInit&&this.onInit()},h.prototype.update=function(a){a=a||!1,a&&(this.min=parseFloat(this.$element[0].getAttribute("min")||0),this.max=parseFloat(this.$element[0].getAttribute("max")||100),this.value=parseFloat(this.$element[0].value||this.min+(this.max-this.min)/2),this.step=parseFloat(this.$element[0].getAttribute("step")||1)),this.handleWidth=g(this.$handle[0],"offsetWidth"),this.rangeWidth=g(this.$range[0],"offsetWidth"),this.maxHandleX=this.rangeWidth-this.handleWidth,this.grabX=this.handleWidth/2,this.position=this.getPositionFromValue(this.value),this.$element[0].disabled?this.$range.addClass(this.options.disabledClass):this.$range.removeClass(this.options.disabledClass),this.setPosition(this.position)},h.prototype.handleDown=function(a){if(a.preventDefault(),this.$document.on(this.moveEvent,this.handleMove),this.$document.on(this.endEvent,this.handleEnd),!((" "+a.target.className+" ").replace(/[\n\t]/g," ").indexOf(this.options.handleClass)>-1)){var b=this.getRelativePosition(a),c=this.$range[0].getBoundingClientRect().left,d=this.getPositionFromNode(this.$handle[0])-c;this.setPosition(b-this.grabX),b>=d&&b<d+this.handleWidth&&(this.grabX=b-d)}},h.prototype.handleMove=function(a){a.preventDefault();var b=this.getRelativePosition(a);this.setPosition(b-this.grabX)},h.prototype.handleEnd=function(a){a.preventDefault(),this.$document.off(this.moveEvent,this.handleMove),this.$document.off(this.endEvent,this.handleEnd),this.$element.trigger("change",{origin:this.identifier}),this.onSlideEnd&&"function"==typeof this.onSlideEnd&&this.onSlideEnd(this.position,this.value)},h.prototype.cap=function(a,b,c){return b>a?b:a>c?c:a},h.prototype.setPosition=function(a){var b,c;b=this.getValueFromPosition(this.cap(a,0,this.maxHandleX)),c=this.getPositionFromValue(b),this.$fill[0].style.width=c+this.grabX+"px",this.$handle[0].style.left=c+"px",this.setValue(b),this.position=c,this.value=b,this.onSlide&&"function"==typeof this.onSlide&&this.onSlide(c,b)},h.prototype.getPositionFromNode=function(a){for(var b=0;null!==a;)b+=a.offsetLeft,a=a.offsetParent;return b},h.prototype.getRelativePosition=function(a){var b=this.$range[0].getBoundingClientRect().left,c=0;return"undefined"!=typeof a.pageX?c=a.pageX:"undefined"!=typeof a.originalEvent.clientX?c=a.originalEvent.clientX:a.originalEvent.touches&&a.originalEvent.touches[0]&&"undefined"!=typeof a.originalEvent.touches[0].clientX?c=a.originalEvent.touches[0].clientX:a.currentPoint&&"undefined"!=typeof a.currentPoint.x&&(c=a.currentPoint.x),c-b},h.prototype.getPositionFromValue=function(a){var b,c;return b=(a-this.min)/(this.max-this.min),c=b*this.maxHandleX},h.prototype.getValueFromPosition=function(a){var b,c;return b=a/(this.maxHandleX||1),c=this.step*Math.round(b*(this.max-this.min)/this.step)+this.min,Number(c.toFixed(this.toFixed))},h.prototype.setValue=function(a){a!==this.value&&this.$element.val(a).trigger("input",{origin:this.identifier})},h.prototype.destroy=function(){this.$document.off("."+this.identifier),this.$window.off("."+this.identifier),this.$element.off("."+this.identifier).removeAttr("style").removeData("plugin_"+i),this.$range&&this.$range.length&&this.$range[0].parentNode.removeChild(this.$range[0])},a.fn[i]=function(b){var c=Array.prototype.slice.call(arguments,1);return this.each(function(){var d=a(this),e=d.data("plugin_"+i);e||d.data("plugin_"+i,e=new h(this,b)),"string"==typeof b&&e[b].apply(e,c)})}});

