#ifndef __ALI_LINKEDGE_WSA_H__
#define __ALI_LINKEDGE_WSA_H__


#if defined(__cplusplus) /* If this is a C++ compiler, use C linkage */
extern "C" {
#endif

typedef struct{
    /*阿里云LinkEdge URL 以实际为准. ws前缀表示非加密方式, wss表示加密方式. 
     * 比如: wss://linkedge.iot.aliyun.com:7682, 
     * 具体以项目为准.*/
    const char                *url;
    /*连接超时时间, 单位为秒. 
     * WSA_init接口阻塞调用，直到连接成功或超时, 连接成功后, 若 timeout 时间内，
     * 未与 LinkEdge 有数据交互, 将重置当前链路, 建议设置成心跳间隔的1.5倍.*/
    int                 timeout;
    /* 根证书存放的绝对路径, 当前版本暂不支持直接传递证书内容, 故需要设备支持文件系统.
     * 当链路加密时, 该值是必须的, 用来校验云端的合法性, 由阿里提供, 具体以项目为准. */
    const char                *ca_path;
    /*设备公钥证书的绝对路径, 当前版本暂不支持直接传递证书内容, 故需要设备支持文件系统.
     * 当链路双向认证时, 该值是必须的, 用来完成TLS握手.*/
    const char                *cert_path;
    /*私钥证书路径的绝对路径, 当前版本暂不支持直接传递证书内容, 故需要设备支持文件系统.
     * 当链路双向认证时, 该值是必须的, 用来完成TLS握手.*/
    const char                *key_path;
}conn_info;

/*
 * SDK 初始化接口, 初始化完成后将与LinkEdge建立长连接通道, 并能根据网络情况自动重连.
 * 
 * 该接口必须在@WSA_register_callback之后再调用， 否者可能导致无法准确获取最新网络状态.
 *
 * info: 参考@conn_info结构.
 *  
 * 阻塞接口, 成功返回0，失败返回错误错误码
 */
int WSA_init(const conn_info *info);

typedef struct
{
    /*由设备特征值组成的唯一描述信息,只能由字母和数字组成, 必须保证每个待接入设备名称不一致*/
    const char *device_id;
    /*产品唯一描述信息, 由阿里提供, 在设备 profile 里也可以查得到.*/
    const char *product_key;
    /*设备 profile 的 md5 值*/
    const char *product_md5;
    /*设备 profile, 由阿里提供, 描述了设备的能力*/
    const char *profile;
}WSA_register_param;

/*
 * 注册并上线设备, 设备只有注册且上线后，才能被 LinkEdge 识别.
 *
 * 若需要注册多个设备，则多次调用该接口即可
 * 该接口是非阻塞的，接口返回并不是说明注册成功，而是表明设备注册消息成功放到了消息发送队列.
 * 开发者无需关注是否注册成功，只要在连接状态异常时，调用本接口即可。
 * @WSA_register_param 具体参数 
 *
 * 非阻塞接口,成功返回0, 失败返回错误码
 */
int WSA_register_online_device(const WSA_register_param *param);


/*
 * 下线设备，假如设备工作在不正常的状态或设备退出前，可以先下线设备，这样LinkEdge就不会继续下发消息到设备侧.
 *
 * device_id: 每个子设备具有不同的设备名，与设备注册时填入的device_id保持一致.
 * @product_key: 产品唯一描述信息, 由阿里提供, 在设备 profile 里也可以查得到.
 *
 * 非阻塞接口, 成功返回0,  失败返回错误码
 *
 */
int WSA_offline_device(const char *device_id, const char *product_key);


typedef enum{
    TYPE_INT = 0,			//整型
    TYPE_BOOL,   			//布尔型
    TYPE_STRING,			//字符串型
    TYPE_FLOAT,				//浮点数
}WSA_Data_Type;

/*属性或事件名的最大长度*/
#define MAX_LENGTH_PARAM_NAME              64

/*属性值或事件参数的最大长度*/
#define MAX_LENGTH_PARAM_VALUE             1024
typedef struct{
    char                        name[MAX_LENGTH_PARAM_NAME];            //属性或事件名
    char                        value[MAX_LENGTH_PARAM_VALUE];          //属性值
    WSA_Data_Type               type;                                   //值类型，需要跟设备 profile 中保持一致.
}WSA_Data;

/*
 * 上报属性, 设备具有的属性在设备能力描述 profile 里有规定。
 *
 * 上报属性，可以上报一个，也可以多个一起上报.
 *
 * properties: @WSA_Data , 属性数组地址
 * count:       本次上报属性个数.
 *
 * 非阻塞接口, 成功返回0,  失败返回错误码
 *
 */
int WSA_report_properties(const char *device_id, const char *product_key, const WSA_Data properties[], int count);

/*
 * 上报事件, 设备具有的事件上报能力在设备 profile 里有约定.
 *
 * 上报事件，单次只能上报一个事件.
 * 
 * event_name:       事件名称
 * data: @WSA_Data , 事件参数数组地址
 * count:            参数个数.
 *
 * 非阻塞接口, 成功返回0,  失败返回错误码
 *
 */
int WSA_report_event(const char *device_id, const char *product_key, const char *event_name, const WSA_Data data[], int count);

typedef enum{
    WSA_CONNECTION_SUCCESS = 0,   //云端连接成功. 
    WSA_CONNECTION_FAILED         //云端连接失败.
}WSA_STATE;

typedef struct{
    /*
     * 获取属性的回调函数, LinkEdge 需要获取某个设备的属性时，SDK 会调用该接口间接获取到数据并封装成固定格式后回传给 LinkEdge.
     * 开发者需要根据设备名和属性名，将属性值获取并以@WSA_Data格式返回。 
	 *
     * @device_id:   LinkEdge 需要获取的具体某个设备的属性，该 device_id 与设备注册时填入的device_id一致.
	 * @property:    LinkEdge 需要获取的设备的具体某个属性名.
     * @output:      开发者需要将属性值更新到output中.
	 * @usr_data:	 注册回调函数时，用户传递的私有数据. 若无，则可以传NULL.
     *
     * 若获取成功则返回0, 失败则返回错误码.
     * */
    int (*cb_get_property)(const char *device_id, const char *property, WSA_Data *output, void *usr_data);
	/* 获取属性回调函数的用户私有数据, 在接口被调用时, 该数据会传递过去*/
    void *usr_data_get_property;
	/*
     * 设置属性的回调函数, LinkEdge 需要设置某个设备的属性时，SDK 会调用该接口将具体的属性值传递给应用程序, 开发者需要在本回调
     * 函数里将属性设置到设备上。 
	 *
     * @device_id:   LinkEdge 需要设置的具体某个设备的属性，该 device_id 与设备注册时填入的device_id一致.
	 * @property:    LinkEdge 需要设置的设备的具体某个属性名和值.
	 * @usr_data:	 注册回调函数时，用户传递的私有数据.
     * */
    int (*cb_set_property)(const char *device_id, const WSA_Data *property, void *usr_data);
	/* 设置属性回调函数的用户私有数据, 在接口被调用时, 该数据会传递过去*/
    void *usr_data_set_property;
	/*
     * 服务调用的回调函数, LinkEdge 需要调用某个设备的服务时，SDK 会调用该接口将具体的服务参数传递给应用程序, 开发者需要在本回调
     * 函数里调用具体的服务，并将服务返回值按照设备 profile 里指定的格式返回。 
	 *
     * @device_id:   LinkEdge 需要调用的具体某个设备的服务，该 device_id 与设备注册时填入的device_id一致.
	 * @method_name: LinkEdge 需要调用的设备的具体某个服务名.
	 * @data:        LinkEdge 需要调用的设备的具体某个服务参数, 参数与设备 profile 中保持一致.
	 * @count:       LinkEdge 需要调用的设备的具体某个服务个数.
     * @output:      开发者需要将服务调用的返回值，按照设备 profile 中规定的服务格式返回到output中.
     * @output_len:  @output 的缓冲区长度.
	 * @usr_data:	 注册回调函数时，用户传递的私有数据.
     * */
    int (*cb_method_call)(const char *device_id, const char *method_name, const WSA_Data *data, int count, char *output, int output_len, void *usr_data);
	/* 服务回调函数的用户私有数据, 在接口被调用时, 该数据会传递过去*/
    void *usr_data_method_call;
	/*
     * 连接状态的回调函数, 目前提供连接建立成功或连接关闭的回调。连接建立成功后，开发者可以根据需要做一些初始化操作; 连接失败后开发者 
	 * 需要重新注册/上线设备。
     * @state:      参考@WSA_STATE
	 * @usr:		 注册回调函数时，用户传递的私有数据.
     * */	
    void (*cb_state_changed)(WSA_STATE state, void *usr_data);
    void *usr_data_state_changed;
}WSA_Callback;

/*
 * 注册回调接口
 *
 * LinkEdge 需要主动调用设备能力时，SDK 通过回调函数获取设备属性/设置设备属性及调用设备服务.
 * cb: @WSA_Callback 
 * 
 * 成功返回0,  失败返回错误码
 *
 */
int WSA_register_callback(const WSA_Callback *cb);

/*
 * 模块退出
 *
 * 程序退出前，释放内存。 
 * 
 * 成功返回0,  失败返回错误码
 *
 */
int WSA_destroy();

/*
 * 设置模块日志等级
 *
 * 日志分为4级: 0级Debug, 1级Info, 2级WARN, 3级ERROR.
 *
 * @lvl: 仅在终端打印高于设置等级的日志.  
 *
 * 默认日志等级是3级ERROR.
 */
void WSA_set_log_level(int lvl);

#if defined(__cplusplus) /* If this is a C++ compiler, use C linkage */
}
#endif

#endif


