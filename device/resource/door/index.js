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
        control(type);
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
        startScanfWiFi();
    });

    /**
     * 保存门配置
     */
    $('.d-btn').on('click', function(){
        setDoorConfig();
    });
	
    /**
     * wifi列表名称获取
     */
	
	$(".wifi-infro-item").click(function(){
		//方法一 find查找所有的子元素，会一直查找，跨层级查找 
		var choose_name_1= $(this).find(".wifi-infro-ssid").html();
		console.log("choose_name_1-------"+choose_name_1);
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
		$(".wifi_scan_ret_list").niceScroll({cursorcolor:"#cccccc"});
		
        //getSafeConfig();
        getWiFiConfig();
        //getDoorConfig();

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
	 *扫描wifi
	 */
	 function startScanfWiFi(){
        $('.wifi_scan_ret_list').show();
        setTimeout(function(){
            $('.wifi_scan_ret_list').hide()
        }, 8000);
		 
	 }
    /**
     * 一般操作
     */
    function control(type){
        var arr = {};
        arr['type'] = 'Control';
        arr['data'] = {};
        arr['data']['ControlType'] = type;

        showLoad();

        $.ajax({
            'url' : '/command',
            'type' : 'post',
            'dataType' : 'json',
            'data' : JSON.stringify(arr),
            'success' : function(res){
                hideLoad();
                var msg = type == 0 ? '1' : (type == 1 ? '2' : (type == 2 ? '3' : '4'));
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
