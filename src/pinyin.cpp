// This file is part of QIpMsg.
//
// QIpMsg is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// QIpMsg is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with QIpMsg.  If not, see <http://www.gnu.org/licenses/>.
//

#include "pinyin.h"

#include <QChar>
#include <QHash>
#include <QStringList>
#include <QTextCodec>

namespace {

// GB2312 first-letter ranges. Public-domain table used by many Chinese IM
// clients (no third-party library). Maps a GB2312 code to an initial A–Z.
char gb2312Initial(quint16 gb)
{
    static const struct {
        quint16 start;
        quint16 end;
        char letter;
    } ranges[] = {
        {0xB0A1, 0xB0C4, 'A'},
        {0xB0C5, 0xB2C0, 'B'},
        {0xB2C1, 0xB4ED, 'C'},
        {0xB4EE, 0xB6E9, 'D'},
        {0xB6EA, 0xB7A1, 'E'},
        {0xB7A2, 0xB8C0, 'F'},
        {0xB8C1, 0xB9FD, 'G'},
        {0xB9FE, 0xBBF6, 'H'},
        {0xBBF7, 0xBFA5, 'J'},
        {0xBFA6, 0xC0AB, 'K'},
        {0xC0AC, 0xC2E7, 'L'},
        {0xC2E8, 0xC4C2, 'M'},
        {0xC4C3, 0xC5B5, 'N'},
        {0xC5B6, 0xC5BD, 'O'},
        {0xC5BE, 0xC6D9, 'P'},
        {0xC6DA, 0xC8BA, 'Q'},
        {0xC8BB, 0xC8F5, 'R'},
        {0xC8F6, 0xCBF9, 'S'},
        {0xCBFA, 0xCDD9, 'T'},
        {0xCDDA, 0xCEF3, 'W'},
        {0xCEF4, 0xD1B8, 'X'},
        {0xD1B9, 0xD4D0, 'Y'},
        {0xD4D1, 0xD7F9, 'Z'}
    };

    const int count = static_cast<int>(sizeof(ranges) / sizeof(ranges[0]));
    for (int i = 0; i < count; ++i) {
        if (gb >= ranges[i].start && gb <= ranges[i].end) {
            return ranges[i].letter;
        }
    }
    return 0;
}

QTextCodec *gbkCodec()
{
    QTextCodec *codec = QTextCodec::codecForName("GBK");
    if (!codec) {
        codec = QTextCodec::codecForName("GB2312");
    }
    return codec;
}

char charInitial(QChar ch)
{
    if (ch.unicode() < 128) {
        if (ch.isLetterOrNumber()) {
            return ch.toLower().toLatin1();
        }
        return 0;
    }

    QTextCodec *codec = gbkCodec();
    if (!codec) {
        return 0;
    }
    const QByteArray gb = codec->fromUnicode(QString(ch));
    if (gb.size() < 2) {
        return 0;
    }
    const quint16 code = (static_cast<quint8>(gb.at(0)) << 8)
            | static_cast<quint8>(gb.at(1));
    return gb2312Initial(code);
}

const QHash<uint, QString> &fullTable()
{
    static QHash<uint, QString> table;
    if (!table.isEmpty()) {
        return table;
    }

    // Common surnames + frequent given-name / UI words. Public readings.
    static const char *pairs[] = {
        "赵","zhao", "钱","qian", "孙","sun", "李","li", "周","zhou",
        "吴","wu", "郑","zheng", "王","wang", "冯","feng", "陈","chen",
        "褚","chu", "卫","wei", "蒋","jiang", "沈","shen", "韩","han",
        "杨","yang", "朱","zhu", "秦","qin", "尤","you", "许","xu",
        "何","he", "吕","lv", "施","shi", "张","zhang", "孔","kong",
        "曹","cao", "严","yan", "华","hua", "金","jin", "魏","wei",
        "陶","tao", "姜","jiang", "戚","qi", "谢","xie", "邹","zou",
        "喻","yu", "柏","bai", "水","shui", "窦","dou", "章","zhang",
        "云","yun", "苏","su", "潘","pan", "葛","ge", "奚","xi",
        "范","fan", "彭","peng", "郎","lang", "鲁","lu", "韦","wei",
        "昌","chang", "马","ma", "苗","miao", "凤","feng", "花","hua",
        "方","fang", "俞","yu", "任","ren", "袁","yuan", "柳","liu",
        "酆","feng", "鲍","bao", "史","shi", "唐","tang", "费","fei",
        "廉","lian", "岑","cen", "薛","xue", "雷","lei", "贺","he",
        "倪","ni", "汤","tang", "滕","teng", "殷","yin", "罗","luo",
        "毕","bi", "郝","hao", "邬","wu", "安","an", "常","chang",
        "乐","le", "于","yu", "时","shi", "傅","fu", "皮","pi",
        "卞","bian", "齐","qi", "康","kang", "伍","wu", "余","yu",
        "元","yuan", "卜","bu", "顾","gu", "孟","meng", "平","ping",
        "黄","huang", "和","he", "穆","mu", "萧","xiao", "尹","yin",
        "姚","yao", "邵","shao", "湛","zhan", "汪","wang", "祁","qi",
        "毛","mao", "禹","yu", "狄","di", "米","mi", "贝","bei",
        "明","ming", "臧","zang", "计","ji", "伏","fu", "成","cheng",
        "戴","dai", "谈","tan", "宋","song", "茅","mao", "庞","pang",
        "熊","xiong", "纪","ji", "舒","shu", "屈","qu", "项","xiang",
        "祝","zhu", "董","dong", "梁","liang", "杜","du", "阮","ruan",
        "蓝","lan", "闵","min", "席","xi", "季","ji", "麻","ma",
        "强","qiang", "贾","jia", "路","lu", "娄","lou", "危","wei",
        "江","jiang", "童","tong", "颜","yan", "郭","guo", "梅","mei",
        "盛","sheng", "林","lin", "刁","diao", "钟","zhong", "徐","xu",
        "邱","qiu", "骆","luo", "高","gao", "夏","xia", "蔡","cai",
        "田","tian", "樊","fan", "胡","hu", "凌","ling", "霍","huo",
        "虞","yu", "万","wan", "支","zhi", "柯","ke", "昝","zan",
        "管","guan", "卢","lu", "莫","mo", "经","jing", "房","fang",
        "裘","qiu", "缪","miao", "干","gan", "解","xie", "应","ying",
        "宗","zong", "丁","ding", "宣","xuan", "贲","ben", "邓","deng",
        "郁","yu", "单","shan", "杭","hang", "洪","hong", "包","bao",
        "诸","zhu", "左","zuo", "石","shi", "崔","cui", "吉","ji",
        "钮","niu", "龚","gong", "程","cheng", "嵇","ji", "邢","xing",
        "滑","hua", "裴","pei", "陆","lu", "荣","rong", "翁","weng",
        "荀","xun", "羊","yang", "於","yu", "惠","hui", "甄","zhen",
        "曲","qu", "家","jia", "封","feng", "芮","rui", "羿","yi",
        "储","chu", "靳","jin", "汲","ji", "邴","bing", "糜","mi",
        "松","song", "井","jing", "段","duan", "富","fu", "巫","wu",
        "乌","wu", "焦","jiao", "巴","ba", "弓","gong", "牧","mu",
        "隗","kui", "山","shan", "谷","gu", "车","che", "侯","hou",
        "宓","mi", "蓬","peng", "全","quan", "郗","xi", "班","ban",
        "仰","yang", "秋","qiu", "仲","zhong", "伊","yi", "宫","gong",
        "宁","ning", "仇","qiu", "栾","luan", "暴","bao", "甘","gan",
        "钭","tou", "厉","li", "戎","rong", "祖","zu", "武","wu",
        "符","fu", "刘","liu", "景","jing", "詹","zhan", "束","shu",
        "龙","long", "叶","ye", "幸","xing", "司","si", "韶","shao",
        "郜","gao", "黎","li", "蓟","ji", "薄","bo", "印","yin",
        "宿","su", "白","bai", "怀","huai", "蒲","pu", "邰","tai",
        "从","cong", "鄂","e", "索","suo", "咸","xian", "籍","ji",
        "赖","lai", "卓","zhuo", "蔺","lin", "屠","tu", "蒙","meng",
        "池","chi", "乔","qiao", "阴","yin", "鬱","yu", "胥","xu",
        "能","neng", "苍","cang", "双","shuang", "闻","wen", "莘","shen",
        "党","dang", "翟","zhai", "谭","tan", "贡","gong", "劳","lao",
        "逄","pang", "姬","ji", "申","shen", "扶","fu", "堵","du",
        "冉","ran", "宰","zai", "郦","li", "雍","yong", "郤","xi",
        "璩","qu", "桑","sang", "桂","gui", "濮","pu", "牛","niu",
        "寿","shou", "通","tong", "边","bian", "扈","hu", "燕","yan",
        "冀","ji", "郏","jia", "浦","pu", "尚","shang", "农","nong",
        "温","wen", "别","bie", "庄","zhuang", "晏","yan", "柴","chai",
        "瞿","qu", "阎","yan", "充","chong", "慕","mu", "连","lian",
        "茹","ru", "习","xi", "宦","huan", "艾","ai", "鱼","yu",
        "容","rong", "向","xiang", "古","gu", "易","yi", "慎","shen",
        "戈","ge", "廖","liao", "庾","yu", "终","zhong", "暨","ji",
        "居","ju", "衡","heng", "步","bu", "都","du", "耿","geng",
        "满","man", "弘","hong", "匡","kuang", "国","guo", "文","wen",
        "寇","kou", "广","guang", "禄","lu", "阙","que", "东","dong",
        "欧","ou", "殳","shu", "沃","wo", "利","li", "蔚","wei",
        "越","yue", "夔","kui", "隆","long", "师","shi", "巩","gong",
        "厍","she", "聂","nie", "晁","chao", "勾","gou", "敖","ao",
        "融","rong", "冷","leng", "訾","zi", "辛","xin", "阚","kan",
        "那","na", "简","jian", "饶","rao", "空","kong", "曾","zeng",
        "母","mu", "沙","sha", "乜","nie", "养","yang", "鞠","ju",
        "须","xu", "丰","feng", "巢","chao", "关","guan", "蒯","kuai",
        "相","xiang", "查","zha", "后","hou", "荆","jing", "红","hong",
        "游","you", "竺","zhu", "权","quan", "逯","lu", "盖","gai",
        "益","yi", "桓","huan", "公","gong",
        "一","yi", "二","er", "三","san", "四","si", "五","wu",
        "六","liu", "七","qi", "八","ba", "九","jiu", "十","shi",
        "小","xiao", "大","da", "中","zhong", "人","ren", "我","wo",
        "你","ni", "他","ta", "她","ta", "们","men", "的","de",
        "是","shi", "了","le", "在","zai", "有","you", "和","he",
        "就","jiu", "不","bu", "这","zhe", "个","ge", "上","shang",
        "来","lai", "到","dao", "说","shuo", "会","hui", "可","ke",
        "以","yi", "要","yao", "出","chu", "也","ye", "对","dui",
        "生","sheng", "工","gong", "作","zuo", "机","ji", "器","qi",
        "好","hao", "新","xin", "老","lao", "朋","peng", "友","you",
        "部","bu", "门","men", "组","zu", "主","zhu", "机","ji",
        "名","ming", "称","cheng", "备","bei", "注","zhu",
        "隐","yin", "身","shen", "搜","sou", "索","suo",
        0
    };

    for (int i = 0; pairs[i] != 0; i += 2) {
        const QString han = QString::fromUtf8(pairs[i]);
        const QString py = QString::fromLatin1(pairs[i + 1]);
        if (!han.isEmpty()) {
            table.insert(han.at(0).unicode(), py);
        }
    }
    return table;
}

} // namespace

QString Pinyin::initials(const QString &text)
{
    QString out;
    for (int i = 0; i < text.size(); ++i) {
        const char letter = charInitial(text.at(i));
        if (letter) {
            out.append(QChar(letter));
        }
    }
    return out;
}

QString Pinyin::fullJoined(const QString &text)
{
    const QHash<uint, QString> &table = fullTable();
    QString out;
    for (int i = 0; i < text.size(); ++i) {
        const QChar ch = text.at(i);
        const QString py = table.value(ch.unicode());
        if (!py.isEmpty()) {
            out.append(py);
            continue;
        }
        const char letter = charInitial(ch);
        if (letter) {
            out.append(QChar(letter));
        } else if (ch.unicode() < 128 && ch.isLetterOrNumber()) {
            out.append(ch.toLower());
        }
    }
    return out;
}

bool Pinyin::matches(const QString &text, const QString &needle)
{
    const QString n = needle.trimmed();
    if (n.isEmpty()) {
        return true;
    }
    if (text.contains(n, Qt::CaseInsensitive)) {
        return true;
    }
    const QString lowered = n.toLower();
    if (initials(text).contains(lowered, Qt::CaseInsensitive)) {
        return true;
    }
    if (fullJoined(text).contains(lowered, Qt::CaseInsensitive)) {
        return true;
    }
    return false;
}

bool Pinyin::matchesAny(const QStringList &fields, const QString &needle)
{
    foreach (const QString &field, fields) {
        if (matches(field, needle)) {
            return true;
        }
    }
    return false;
}
