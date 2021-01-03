#!/bin/bash


# Cleanup the produced files for testing various cases for division
for divu in "1" "2" "3" "4" "5" "6" "7" "8" "9" "10" "11" "15" "23" "31" "49" "57" "63" "111" "127" "255" "351" "641" "734" "1000" "345345"
do
  rm -rf kdiv_u32_p_${divu}.nac
  rm -rf kdiv_u32_p_${divu}.c
done

for divs in "1" "2" "3" "4" "5" "6" "7" "8" "9" "10" "11" "15" "23" "31" "49" "57" "63" "111" "127" "255" "351" "641" "734" "1000" "345345"
do
  rm -rf kdiv_s32_m_${divs}.nac 
  rm -rf kdiv_s32_m_${divs}.c
  rm -rf kdiv_s32_p_${divs}.nac 
  rm -rf kdiv_s32_p_${divs}.c
done
