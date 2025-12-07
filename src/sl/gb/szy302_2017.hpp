//
// Created by Arjen on 2025/1/24.
//

#ifndef SZY302_2017_HPP
#define SZY302_2017_HPP

#include <string>
#include <optional>
#include <boost/date_time.hpp>
#include <nlohmann/json.hpp>

#include "../../core/json_ser_deser.hpp"

class base_table
{

public:
    /**
     * 时间戳
     */
    std::tm ts;

    /**
     * 备注
     */
    std::string nt;

};

class wr_mp_q_r
{

public:

    explicit wr_mp_q_r() = default;

    /**
     * 监测点代码
     */
    std::string mpcd;

    /**
     * 时间戳
     */
    std::tm ts;

    /**
     * 特殊区域数据
     */
    bool spe_reg_data;

    /**
     * 累计水量
     */
    double acc_w;

    /**
     * 观测流量
     */
    double mp_q;

    /**
     * 时间
     */
    std::tm tm;

};

class wr_rtu_mon_r: public base_table
{

public:
    /**
     * RTU 代码
     */
    std::string rtu_cd;

    /**
     * 记录时间
     */
    std::tm rec_tm;

    /**
     * 蓄电池电压
     */
    double bat_v;

    /**
     * 蓄电池电压报警
     */
    bool bat_v_warm;

    /**
     * 交流电工作状态
     */
    bool acpwr_cond;

    /**
     * 太阳能电源工作状态
     */
    bool solpwr_cond;

    /**
     * RTU 可控状态
     */
    bool rtu_cond;

    /**
     * IC 卡状态
     */
    bool ic_cond;

    /**
     * 存储器状态
     */
    bool mem_cond;

    /**
     * 终端箱门状态报警
     */
    bool chas_door_cond;

    /**
     * 其他报警
     */
    bool oth_warm;

};

class work_state: public wr_rtu_mon_r
{

public:

    work_state(short waring_status);

    /**
     * 水位超限报警
     */
    bool z;

    /**
     * 流量超限报警
     */
    bool q;

    /**
     * 水质超限报警
     */
    bool wq;

    /**
     * 流量仪表故障报警
     */
    bool qd;

    /**
     * 水泵开停状态
     */
    bool wp;

    /**
     * 水位仪表故障报警
     */
    bool zp;

    /**
     * 水压超限报警
     */
    bool h;

    /**
     * 温度超限报警
     */
    bool t;

    /**
     * 定值控制报警
     */
    bool sv;

    /**
     * 剩余水量报警
     */
    bool rwv;

};

namespace wq {

    /**
     * 水质类
     */
    class wq_awqmd_d {

    public:

        /**
         * 构造这个对象，必须要一个站码
         * @param stcd
         */
        explicit wq_awqmd_d(const std::string &stcd);

        std::string _stcd;

        // 采样时间
        boost::posix_time::ptime _spt;

        // 铅
        std::optional<double> _pb;

        // 铜
        std::optional<double> _cu;

        // 六价铬
        std::optional<double> _cr6;

        // 汞
        std::optional<double> _hg;

        // 砷
        std::optional<double> _ars;

        // 氟化物
        std::optional<double> _f;

        // 叶绿素a
        std::optional<double> _chla;

        // 挥发酚
        std::optional<double> _vlph;

        // 总有机碳
        std::optional<double> _toc;

        // 总磷
        std::optional<double> _tp;

        // 硝酸盐氮
        std::optional<double> _no3;

        // 亚硝酸盐氮
        std::optional<double> _no2;

        // 氨氮
        std::optional<double> _nh3n;

        // 总氮
        std::optional<double> _tn;

        // 化学需氧量
        std::optional<double> _codcr;

        // 高锰酸盐指数
        std::optional<double> _codmn;

        // 溶解氧
        std::optional<double> _dox;

        // 浊度
        std::optional<double> _turb;

        // 电导率
        std::optional<double> _cond;

        // pH值
        std::optional<double> _ph;

        // 水温
        std::optional<double> _wt;

        // 锑
        std::optional<double> _sb;

        // 锌
        std::optional<double> _zn;

        // 镉
        std::optional<double> _cd;

        // 化验完成时间
        std::optional<boost::posix_time::ptime> _test_com_tm;

        // 时间戳
        boost::posix_time::ptime _ts;

        // 特殊区域
        std::optional<std::string> _spe_reg_data;

        // 非标准表字段，这里只是拓展属性，交给业务端处理
        std::unordered_map<std::string, std::optional<double>> _expand;

    public:

        friend void to_json(nlohmann::json &j, const wq_awqmd_d &awqmd);

    };
}

#endif //SZY302_2017_HPP
