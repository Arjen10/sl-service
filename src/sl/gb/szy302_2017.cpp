//
// Created by Arjen on 2025/1/24.
//

#include "szy302_2017.hpp"

work_state::work_state(short waring_status)
{
    acpwr_cond = (waring_status >> 15 & 0x1) == 1;
    bat_v_warm = (waring_status >> 14 & 0x1) == 1;
    // 设置其他状态
    z = (waring_status >> 13 & 0x1) == 1;
    q = (waring_status >> 12 & 0x1) == 1;
    wq = (waring_status >> 11 & 0x1) == 1;
    qd = (waring_status >> 10 & 0x1) == 1;
    wp = (waring_status >> 9 & 0x1) == 1;
    zp = (waring_status >> 8 & 0x1) == 1;
    h = (waring_status >> 7 & 0x1) == 1;
    t = (waring_status >> 6 & 0x1) == 1;
    ic_cond = (waring_status >> 5 & 0x1) == 1;
    sv = (waring_status >> 4 & 0x1) == 1;
    rwv = (waring_status >> 3 & 0x1) == 1;
    ic_cond = (waring_status >> 2 & 0x1) == 1;
    // 计算是否有其他警告
    oth_warm = z || q || wq || qd || wp || zp || h || t || sv || rwv;
}

wq::wq_awqmd_d::wq_awqmd_d(const std::string &stcd) : _stcd(stcd) {
    _ts = boost::posix_time::second_clock::local_time();

}

void wq::to_json(nlohmann::json &j, const wq_awqmd_d &awqmd) {
    j = nlohmann::json::object();
    JSON_FIELD_REF(j, awqmd, _stcd);
    JSON_FIELD_REF(j, awqmd, _spt);
    JSON_FIELD_REF(j, awqmd, _pb);
    JSON_FIELD_REF(j, awqmd, _cu);
    JSON_FIELD_REF(j, awqmd, _cr6);
    JSON_FIELD_REF(j, awqmd, _hg);
    JSON_FIELD_REF(j, awqmd, _ars);
    JSON_FIELD_REF(j, awqmd, _f);
    JSON_FIELD_REF(j, awqmd, _chla);
    JSON_FIELD_REF(j, awqmd, _vlph);
    JSON_FIELD_REF(j, awqmd, _toc);
    JSON_FIELD_REF(j, awqmd, _tp);
    JSON_FIELD_REF(j, awqmd, _no3);
    JSON_FIELD_REF(j, awqmd, _no2);
    JSON_FIELD_REF(j, awqmd, _nh3n);
    JSON_FIELD_REF(j, awqmd, _tn);
    JSON_FIELD_REF(j, awqmd, _codcr);
    JSON_FIELD_REF(j, awqmd, _codmn);
    JSON_FIELD_REF(j, awqmd, _dox);
    JSON_FIELD_REF(j, awqmd, _turb);
    JSON_FIELD_REF(j, awqmd, _cond);
    JSON_FIELD_REF(j, awqmd, _ph);
    JSON_FIELD_REF(j, awqmd, _wt);
    JSON_FIELD_REF(j, awqmd, _sb);
    JSON_FIELD_REF(j, awqmd, _zn);
    JSON_FIELD_REF(j, awqmd, _cd);
    JSON_FIELD_REF(j, awqmd, _test_com_tm);
    JSON_FIELD_REF(j, awqmd, _ts);
    JSON_FIELD_REF(j, awqmd, _spe_reg_data);
    JSON_FIELD_REF(j, awqmd, _extended);
}
