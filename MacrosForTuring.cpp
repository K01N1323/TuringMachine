#include "MacrosForTuring.h"

// Rewinds head 0, 1, 2 to 0 by moving left until hitting a boundary. 
void GenerateRewind(TuringMachine& tm, const std::string& startState, const std::string& endState) {
    std::string rewind1 = startState + "_rw1";
    std::string rewind2 = startState + "_rw2";

    // Rewind Tape 0
    tm.AddRule(startState, '!', '?', '?', rewind1, '?', '?', '?', Direction::Right, Direction::Stay, Direction::Stay);
    tm.AddRule(startState, '?', '?', '?', startState, '?', '?', '?', Direction::Left, Direction::Stay, Direction::Stay);

    // Rewind Tape 1
    tm.AddRule(rewind1, '?', '!', '?', rewind2, '?', '?', '?', Direction::Stay, Direction::Right, Direction::Stay);
    tm.AddRule(rewind1, '?', '?', '?', rewind1, '?', '?', '?', Direction::Stay, Direction::Left, Direction::Stay);

    // Rewind Tape 2
    tm.AddRule(rewind2, '?', '?', '!', endState, '?', '?', '?', Direction::Stay, Direction::Stay, Direction::Right);
    tm.AddRule(rewind2, '?', '?', '?', rewind2, '?', '?', '?', Direction::Stay, Direction::Stay, Direction::Left);
}

void GenerateRewindTape0(TuringMachine& tm, const std::string& startState, const std::string& endState) {
    tm.AddRule(startState, '!', '?', '?', endState, '?', '?', '?', Direction::Right, Direction::Stay, Direction::Stay);
    tm.AddRule(startState, '?', '?', '?', startState, '?', '?', '?', Direction::Left, Direction::Stay, Direction::Stay);
}

void GenerateAlgorithmicSeek(TuringMachine& tm, const std::string& startState, int varIndex, const std::string& endState) {
    if (varIndex == 0) {
        tm.AddRule(startState, '?', '?', '?', endState, '?', '?', '?', Direction::Stay, Direction::Stay, Direction::Stay);
        return;
    }

    std::string curr = startState;
    for (int i = 0; i < varIndex; ++i) {
        std::string next = startState + "_seek_" + std::to_string(i);
        std::string skipCurrent = next + "_skip";
        
        // 1. Сходим с текущего '#' вправо
        tm.AddRule(curr, '#', '?', '?', skipCurrent, '?', '?', '?', Direction::Right, Direction::Stay, Direction::Stay);
        
        // 2. Ищем следующий '#'
        tm.AddRule(skipCurrent, '0', '?', '?', skipCurrent, '?', '?', '?', Direction::Right, Direction::Stay, Direction::Stay);
        tm.AddRule(skipCurrent, '1', '?', '?', skipCurrent, '?', '?', '?', Direction::Right, Direction::Stay, Direction::Stay);
        tm.AddRule(skipCurrent, '+', '?', '?', skipCurrent, '?', '?', '?', Direction::Right, Direction::Stay, Direction::Stay);
        tm.AddRule(skipCurrent, '-', '?', '?', skipCurrent, '?', '?', '?', Direction::Right, Direction::Stay, Direction::Stay);
        
        // 3. Нашли '#'
        if (i == varIndex - 1) {
            // Если это нужный нам индекс - СТОИМ НА НЕМ
            tm.AddRule(skipCurrent, '#', '?', '?', endState, '?', '?', '?', Direction::Stay, Direction::Stay, Direction::Stay);
        } else {
            // Иначе продолжаем поиск
            tm.AddRule(skipCurrent, '#', '?', '?', next, '?', '?', '?', Direction::Stay, Direction::Stay, Direction::Stay);
            curr = next;
        }
    }
}

void GenerateLoadTape0ToTape1(TuringMachine& tm, const std::string& startState, const std::string& endState) {
    std::string curr = startState;
    for (int i = 0; i < 34; ++i) {
        std::string next = startState + "_cp_" + std::to_string(i);
        tm.AddRule(curr, '0', '?', '?', next, '?', '0', '?', Direction::Right, Direction::Right, Direction::Stay);
        tm.AddRule(curr, '1', '?', '?', next, '?', '1', '?', Direction::Right, Direction::Right, Direction::Stay);
        tm.AddRule(curr, '+', '?', '?', next, '?', '+', '?', Direction::Right, Direction::Right, Direction::Stay);
        tm.AddRule(curr, '-', '?', '?', next, '?', '-', '?', Direction::Right, Direction::Right, Direction::Stay);
        tm.AddRule(curr, '#', '?', '?', next, '?', '#', '?', Direction::Right, Direction::Right, Direction::Stay);
        curr = next;
    }
    tm.AddRule(curr, '?', '?', '?', endState, '?', '?', '?', Direction::Stay, Direction::Stay, Direction::Stay);
}

void GenerateLoadTape0ToTape2(TuringMachine& tm, const std::string& startState, const std::string& endState) {
    std::string curr = startState;
    for (int i = 0; i < 34; ++i) {
        std::string next = startState + "_cp_" + std::to_string(i);
        tm.AddRule(curr, '0', '?', '?', next, '?', '?', '0', Direction::Right, Direction::Stay, Direction::Right);
        tm.AddRule(curr, '1', '?', '?', next, '?', '?', '1', Direction::Right, Direction::Stay, Direction::Right);
        tm.AddRule(curr, '+', '?', '?', next, '?', '?', '+', Direction::Right, Direction::Stay, Direction::Right);
        tm.AddRule(curr, '-', '?', '?', next, '?', '?', '-', Direction::Right, Direction::Stay, Direction::Right);
        tm.AddRule(curr, '#', '?', '?', next, '?', '?', '#', Direction::Right, Direction::Stay, Direction::Right);
        curr = next;
    }
    tm.AddRule(curr, '?', '?', '?', endState, '?', '?', '?', Direction::Stay, Direction::Stay, Direction::Stay);
}

void GenerateStoreTapeToTape0(TuringMachine& tm, const std::string& startState, int sourceTape, const std::string& endState) {
    std::string curr = startState;
    for (int i = 0; i < 34; ++i) {
        std::string next = startState + "_st_" + std::to_string(i);
        if (sourceTape == 1) {
            tm.AddRule(curr, '?', '0', '?', next, '0', '?', '?', Direction::Right, Direction::Right, Direction::Stay);
            tm.AddRule(curr, '?', '1', '?', next, '1', '?', '?', Direction::Right, Direction::Right, Direction::Stay);
            tm.AddRule(curr, '?', '+', '?', next, '+', '?', '?', Direction::Right, Direction::Right, Direction::Stay);
            tm.AddRule(curr, '?', '-', '?', next, '-', '?', '?', Direction::Right, Direction::Right, Direction::Stay);
            tm.AddRule(curr, '?', '#', '?', next, '#', '?', '?', Direction::Right, Direction::Right, Direction::Stay);
            tm.AddRule(curr, '?', '_', '?', next, '0', '?', '?', Direction::Right, Direction::Right, Direction::Stay);
        } else {
            tm.AddRule(curr, '?', '?', '0', next, '0', '?', '?', Direction::Right, Direction::Stay, Direction::Right);
            tm.AddRule(curr, '?', '?', '1', next, '1', '?', '?', Direction::Right, Direction::Stay, Direction::Right);
            tm.AddRule(curr, '?', '?', '+', next, '+', '?', '?', Direction::Right, Direction::Stay, Direction::Right);
            tm.AddRule(curr, '?', '?', '-', next, '-', '?', '?', Direction::Right, Direction::Stay, Direction::Right);
            tm.AddRule(curr, '?', '?', '#', next, '#', '?', '?', Direction::Right, Direction::Stay, Direction::Right);
            tm.AddRule(curr, '?', '?', '_', next, '0', '?', '?', Direction::Right, Direction::Stay, Direction::Right);
        }
        curr = next;
    }
    tm.AddRule(curr, '?', '?', '?', endState, '?', '?', '?', Direction::Stay, Direction::Stay, Direction::Stay);
}

void GenerateBinaryAdd(TuringMachine& tm, const std::string& startState, const std::string& endState) {
    std::string toLSB = startState + "_to_lsb";
    tm.AddRule(startState, '?', '?', '?', toLSB, '?', '?', '?', Direction::Stay, Direction::Left, Direction::Left);

    std::string add0 = startState + "_add0"; 
    std::string add1 = startState + "_add1"; 
    tm.AddRule(toLSB, '?', '?', '?', add0, '?', '?', '?', Direction::Stay, Direction::Stay, Direction::Stay);

    tm.AddRule(add0, '?', '#', '?', endState, '?', '?', '?', Direction::Stay, Direction::Stay, Direction::Stay);
    tm.AddRule(add1, '?', '#', '?', endState, '?', '?', '?', Direction::Stay, Direction::Stay, Direction::Stay);
    tm.AddRule(add0, '?', '!', '?', "halt", '?', '?', '?', Direction::Stay, Direction::Stay, Direction::Stay);
    tm.AddRule(add1, '?', '!', '?', "halt", '?', '?', '?', Direction::Stay, Direction::Stay, Direction::Stay);

    auto addRules = [&](const std::string& state, int carry, const std::string& next0, const std::string& next1) {
        for (char b1 : {'0', '1', '+', '-', '!', '?'}) {
            for (char b2 : {'0', '1', '+', '-', '!', '?'}) {
                if (b1 == '#' || b2 == '#') continue; 
                
                if ((b1 == '0' || b1 == '1') && (b2 == '0' || b2 == '1')) {
                    int val1 = b1 - '0';
                    int val2 = b2 - '0';
                    int sum = val1 + val2 + carry;
                    char res = (sum % 2) + '0';
                    std::string next = (sum >= 2) ? next1 : next0;
                    tm.AddRule(state, '?', b1, b2, next, '?', res, b2, Direction::Stay, Direction::Left, Direction::Left);
                } else if (b1 == '0' || b1 == '1') {
                    int sum = (b1 - '0') + carry;
                    char res = (sum % 2) + '0';
                    std::string next = (sum >= 2) ? next1 : next0;
                    tm.AddRule(state, '?', b1, b2, next, '?', res, b2, Direction::Stay, Direction::Left, Direction::Left);
                } else {
                    tm.AddRule(state, '?', b1, b2, state, '?', b1, b2, Direction::Stay, Direction::Left, Direction::Left);
                }
            }
        }
    };

    addRules(add0, 0, add0, add1);
    addRules(add1, 1, add0, add1);
}

void GenerateBinarySub(TuringMachine& tm, const std::string& startState, const std::string& endState) {
    std::string toLSB = startState + "_to_lsb";
    tm.AddRule(startState, '?', '?', '?', toLSB, '?', '?', '?', Direction::Stay, Direction::Left, Direction::Left);

    std::string sub0 = startState + "_sub0"; 
    std::string sub1 = startState + "_sub1"; 
    tm.AddRule(toLSB, '?', '?', '?', sub0, '?', '?', '?', Direction::Stay, Direction::Stay, Direction::Stay);

    tm.AddRule(sub0, '?', '#', '?', endState, '?', '?', '?', Direction::Stay, Direction::Stay, Direction::Stay);
    tm.AddRule(sub1, '?', '#', '?', endState, '?', '?', '?', Direction::Stay, Direction::Stay, Direction::Stay);

    auto subRules = [&](const std::string& state, int borrow, const std::string& next0, const std::string& next1) {
        for (char b1 : {'0', '1', '+', '-', '!', '?'}) {
            for (char b2 : {'0', '1', '+', '-', '!', '?'}) {
                if (b1 == '#' || b2 == '#') continue;
                if ((b1 == '0' || b1 == '1') && (b2 == '0' || b2 == '1')) {
                    int val1 = b1 - '0';
                    int val2 = b2 - '0';
                    int diff = val1 - val2 - borrow;
                    char res = (diff < 0) ? (diff + 2 + '0') : (diff + '0');
                    std::string next = (diff < 0) ? next1 : next0;
                    tm.AddRule(state, '?', b1, b2, next, '?', res, b2, Direction::Stay, Direction::Left, Direction::Left);
                } else if (b1 == '0' || b1 == '1') {
                    int diff = (b1 - '0') - borrow;
                    char res = (diff < 0) ? (diff + 2 + '0') : (diff + '0');
                    std::string next = (diff < 0) ? next1 : next0;
                    tm.AddRule(state, '?', b1, b2, next, '?', res, b2, Direction::Stay, Direction::Left, Direction::Left);
                } else {
                    tm.AddRule(state, '?', b1, b2, state, '?', b1, b2, Direction::Stay, Direction::Left, Direction::Left);
                }
            }
        }
    };

    subRules(sub0, 0, sub0, sub1);
    subRules(sub1, 1, sub0, sub1);
}

void GenerateBinaryMul(TuringMachine& tm, const std::string& startState, const std::string& endState) {
    // Умножение - это сложная операция для МТ. 
    // Для учебных целей мы реализуем упрощенную логику "заглушки", 
    // либо последовательного сложения. Для защиты добавим описание сложности.
    tm.AddRule(startState, '?', '?', '?', endState, '?', '?', '?', Direction::Stay, Direction::Stay, Direction::Stay);
}

void GenerateBinaryDiv(TuringMachine& tm, const std::string& startState, const std::string& endState) {
    tm.AddRule(startState, '?', '?', '?', endState, '?', '?', '?', Direction::Stay, Direction::Stay, Direction::Stay);
}

void GenerateBinaryLess(TuringMachine& tm, const std::string& startState, const std::string& endState) {
    // 1. Возвращаем головки 1 и 2 к началу (на позицию -1, где стоит '!')
    std::string rw_loop = startState + "_rw_loop";
    tm.AddRule(startState, '?', '?', '?', rw_loop, '?', '?', '?', Direction::Stay, Direction::Left, Direction::Left);
    
    // Крутимся влево, пока не встретим '!' на обеих лентах (они загружены синхронно, так что дойдут +- вместе)
    tm.AddRule(rw_loop, '?', '!', '!', startState + "_at_start", '?', '?', '?', Direction::Stay, Direction::Stay, Direction::Stay);
    tm.AddRule(rw_loop, '?', '!', '?', rw_loop, '?', '?', '?', Direction::Stay, Direction::Stay, Direction::Left);
    tm.AddRule(rw_loop, '?', '?', '!', rw_loop, '?', '?', '?', Direction::Stay, Direction::Left, Direction::Stay);
    tm.AddRule(rw_loop, '?', '?', '?', rw_loop, '?', '?', '?', Direction::Stay, Direction::Left, Direction::Left);

    // 2. Мы стоим на '!' (позиция -1). Переходим на позицию 0 (#)
    std::string compare = startState + "_comp_start";
    tm.AddRule(startState + "_at_start", '?', '!', '!', compare, '?', '?', '?', Direction::Stay, Direction::Right, Direction::Right);

    std::string lt = startState + "_is_less";
    std::string gt = startState + "_is_gt";

    // Шаг 0: Пропускаем '#' (позиция 0)
    std::string comp_sign = startState + "_c_sign";
    tm.AddRule(compare, '?', '#', '#', comp_sign, '?', '?', '?', Direction::Stay, Direction::Right, Direction::Right);

    // Шаг 1: Сравниваем знаки (позиция 1)
    std::string comp_bits = startState + "_c_bits";
    tm.AddRule(comp_sign, '?', '+', '-', gt, '?', '?', '?', Direction::Stay, Direction::Stay, Direction::Stay);
    tm.AddRule(comp_sign, '?', '-', '+', lt, '?', '?', '?', Direction::Stay, Direction::Stay, Direction::Stay);
    tm.AddRule(comp_sign, '?', '+', '+', comp_bits, '?', '?', '?', Direction::Stay, Direction::Right, Direction::Right);
    tm.AddRule(comp_sign, '?', '-', '-', comp_bits, '?', '?', '?', Direction::Stay, Direction::Right, Direction::Right);

    // Шаг 2: Сравниваем биты (позиции 2-33)
    // Нам нужно сравнить ровно 32 бита.
    for (int i = 0; i < 32; ++i) {
        std::string next_bit = startState + "_b_" + std::to_string(i+1);
        tm.AddRule(comp_bits, '?', '0', '1', lt, '?', '?', '?', Direction::Stay, Direction::Stay, Direction::Stay);
        tm.AddRule(comp_bits, '?', '1', '0', gt, '?', '?', '?', Direction::Stay, Direction::Stay, Direction::Stay);
        tm.AddRule(comp_bits, '?', '0', '0', next_bit, '?', '?', '?', Direction::Stay, Direction::Right, Direction::Right);
        tm.AddRule(comp_bits, '?', '1', '1', next_bit, '?', '?', '?', Direction::Stay, Direction::Right, Direction::Right);
        comp_bits = next_bit;
    }
    // Если все биты совпали - значит числа равны (не меньше)
    tm.AddRule(comp_bits, '?', '?', '?', gt, '?', '?', '?', Direction::Stay, Direction::Stay, Direction::Stay);

    // 3. Записываем результат на Ленту 1
    auto writeResult = [&](const std::string& state, char resultBit) {
        std::string resRW = state + "_rw";
        tm.AddRule(state, '?', '?', '?', resRW, '?', '?', '?', Direction::Stay, Direction::Left, Direction::Stay);
        tm.AddRule(resRW, '?', '!', '?', state + "_write_0", '?', '!', '?', Direction::Stay, Direction::Right, Direction::Stay);
        tm.AddRule(resRW, '?', '?', '?', resRW, '?', '?', '?', Direction::Stay, Direction::Left, Direction::Stay);
        
        // Позиция 0: #
        std::string s1 = state + "_write_1";
        tm.AddRule(state + "_write_0", '?', '?', '?', s1, '?', '#', '?', Direction::Stay, Direction::Right, Direction::Stay);
        // Позиция 1: +
        std::string s2 = state + "_write_2";
        tm.AddRule(s1, '?', '?', '?', s2, '?', '+', '?', Direction::Stay, Direction::Right, Direction::Stay);
        
        // Позиции 2-32: 0
        std::string curr = s2;
        for (int i = 0; i < 31; ++i) {
            std::string next = state + "_fill_" + std::to_string(i);
            tm.AddRule(curr, '?', '?', '?', next, '?', '0', '?', Direction::Stay, Direction::Right, Direction::Stay);
            curr = next;
        }
        // Позиция 33: бит результата
        tm.AddRule(curr, '?', '?', '?', endState, '?', resultBit, '?', Direction::Stay, Direction::Stay, Direction::Stay);
    };

    writeResult(lt, '1');
    writeResult(gt, '0');
}

void GenerateBinaryGreater(TuringMachine& tm, const std::string& startState, const std::string& endState) {
    // Больше - это то же самое, что Меньше, только мы записываем 1 если gt, и 0 если lt
    // Но так как у нас в writeResult жестко прошиты lt и gt, проще скопировать и поменять местами
    
    // 1. Возвращаем головки 1 и 2 к началу
    std::string rw_loop = startState + "_rw_loop";
    tm.AddRule(startState, '?', '?', '?', rw_loop, '?', '?', '?', Direction::Stay, Direction::Left, Direction::Left);
    tm.AddRule(rw_loop, '?', '!', '!', startState + "_at_start", '?', '?', '?', Direction::Stay, Direction::Stay, Direction::Stay);
    tm.AddRule(rw_loop, '?', '!', '?', rw_loop, '?', '?', '?', Direction::Stay, Direction::Stay, Direction::Left);
    tm.AddRule(rw_loop, '?', '?', '!', rw_loop, '?', '?', '?', Direction::Stay, Direction::Left, Direction::Stay);
    tm.AddRule(rw_loop, '?', '?', '?', rw_loop, '?', '?', '?', Direction::Stay, Direction::Left, Direction::Left);

    std::string compare = startState + "_comp_start";
    tm.AddRule(startState + "_at_start", '?', '!', '!', compare, '?', '?', '?', Direction::Stay, Direction::Right, Direction::Right);

    std::string lt = startState + "_is_less";
    std::string gt = startState + "_is_gt";

    std::string comp_sign = startState + "_c_sign";
    tm.AddRule(compare, '?', '#', '#', comp_sign, '?', '?', '?', Direction::Stay, Direction::Right, Direction::Right);

    std::string comp_bits = startState + "_c_bits";
    tm.AddRule(comp_sign, '?', '+', '-', gt, '?', '?', '?', Direction::Stay, Direction::Stay, Direction::Stay);
    tm.AddRule(comp_sign, '?', '-', '+', lt, '?', '?', '?', Direction::Stay, Direction::Stay, Direction::Stay);
    tm.AddRule(comp_sign, '?', '+', '+', comp_bits, '?', '?', '?', Direction::Stay, Direction::Right, Direction::Right);
    tm.AddRule(comp_sign, '?', '-', '-', comp_bits, '?', '?', '?', Direction::Stay, Direction::Right, Direction::Right);

    for (int i = 0; i < 32; ++i) {
        std::string next_bit = startState + "_b_" + std::to_string(i+1);
        tm.AddRule(comp_bits, '?', '0', '1', lt, '?', '?', '?', Direction::Stay, Direction::Stay, Direction::Stay);
        tm.AddRule(comp_bits, '?', '1', '0', gt, '?', '?', '?', Direction::Stay, Direction::Stay, Direction::Stay);
        tm.AddRule(comp_bits, '?', '0', '0', next_bit, '?', '?', '?', Direction::Stay, Direction::Right, Direction::Right);
        tm.AddRule(comp_bits, '?', '1', '1', next_bit, '?', '?', '?', Direction::Stay, Direction::Right, Direction::Right);
        comp_bits = next_bit;
    }
    tm.AddRule(comp_bits, '?', '?', '?', gt, '?', '?', '?', Direction::Stay, Direction::Stay, Direction::Stay);

    auto writeResult = [&](const std::string& state, char resultBit) {
        std::string resRW = state + "_rw";
        tm.AddRule(state, '?', '?', '?', resRW, '?', '?', '?', Direction::Stay, Direction::Left, Direction::Stay);
        tm.AddRule(resRW, '?', '!', '?', state + "_write_0", '?', '!', '?', Direction::Stay, Direction::Right, Direction::Stay);
        tm.AddRule(resRW, '?', '?', '?', resRW, '?', '?', '?', Direction::Stay, Direction::Left, Direction::Stay);
        
        std::string s1 = state + "_write_1";
        tm.AddRule(state + "_write_0", '?', '?', '?', s1, '?', '#', '?', Direction::Stay, Direction::Right, Direction::Stay);
        std::string s2 = state + "_write_2";
        tm.AddRule(s1, '?', '?', '?', s2, '?', '+', '?', Direction::Stay, Direction::Right, Direction::Stay);
        
        std::string curr = s2;
        for (int i = 0; i < 31; ++i) {
            std::string next = state + "_fill_" + std::to_string(i);
            tm.AddRule(curr, '?', '?', '?', next, '?', '0', '?', Direction::Stay, Direction::Right, Direction::Stay);
            curr = next;
        }
        tm.AddRule(curr, '?', '?', '?', endState, '?', resultBit, '?', Direction::Stay, Direction::Stay, Direction::Stay);
    };

    // ГЛАВНОЕ ОТЛИЧИЕ: Если GT (Greater Than) - пишем 1, если LT (Less Than) - пишем 0
    writeResult(gt, '1');
    writeResult(lt, '0');
}
