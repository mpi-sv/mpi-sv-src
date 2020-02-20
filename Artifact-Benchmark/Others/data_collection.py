#!/usr/bin/env python
#-*- coding:utf-8 -*-

import xlrd
from fractions import Fraction

line_map = {"DTG": [1, 2, 3, 4, 5, 6],
            "Matmat-MS": [8],
            "Integrate": [10, 11, 12],
            "Integrate-MS": [14],
            "Diffusion2d": [16, 17, 18, 19, 20, 21],
            "Gauss_elim": [23, 24],
            "Heat": [26, 27, 28, 29, 30, 31],
            "Mandelbrot": [33, 34, 35, 36],
            "Mandelbrot-MS": [38],
            "Sorting-MS": [40],
            "Image_mani": [42, 43],
            "DepSolver": [45],
            "Kfray": [47, 48, 49, 50],
            "Kfray-MS": [52],
            "ClustalW": [54, 55, 56, 57, 58, 59]}

objects = ["DTG", "Matmat-MS", "Integrate", "Integrate-MS", "Diffusion2d",
           "Gauss_elim", "Heat", "Mandelbrot", "Mandelbrot-MS", "Sorting-MS",
           "Image_mani", "DepSolver", "Kfray", "Kfray-MS", "ClustalW"]

time_deadlockfree_SE = {}
time_deadlockfree_SV = {}
time_deadlock_SE = {}
time_deadlock_SV = {}
iteration_deadlockfree_SE = {}
iteration_deadlockfree_SV = {}
iteration_deadlock_SE = {}
iteration_deadlock_SV = {}

time_thread_SE = []
time_thread_SV = []


if __name__ == "__main__":
    file1 = xlrd.open_workbook(r'./average.xls')
    table1 = file1.sheet_by_index(0)
    deadlockfree_SE = 0
    deadlockfree_SV = 0
    deadlock_SE = 0
    deadlock_SV = 0
    for object in objects:
        time_no_SE = []
        time_no_SV = []
        time_yes_SE = []
        time_yes_SV = []
        iteration_no_SE = []
        iteration_no_SV = []
        iteration_yes_SE = []
        iteration_yes_SV = []
        for line in line_map[object]:
            Deadlock_info1 = table1.cell(line, 2).value
            SE_iter1 = table1.cell(line, 3).value
            SV_iter1 = table1.cell(line, 4).value
            SE_time1 = table1.cell(line, 5).value
            SV_time1 = table1.cell(line, 6).value


            if object == "DTG" or object == "Matmat-MS":
                if int(Deadlock_info1) == 0:
                    deadlockfree_SE += 1                    # for percentage (deadlock-free/deadlock num within time limit)
                    deadlockfree_SV += 1
                    time_no_SE.append(float(SE_time1))      # for the timecost ratio between SE and SV
                    time_no_SV.append(float(SV_time1))
                    iteration_no_SE.append(int(SE_iter1))   # for the iteration_no ratio between SE and SV
                    iteration_no_SV.append(int(SV_iter1))

                elif int(Deadlock_info1) == 1:
                    deadlock_SE += 1
                    deadlock_SV += 1
                    time_yes_SE.append(float(SE_time1))
                    time_yes_SV.append(float(SV_time1))
                    iteration_yes_SE.append(int(SE_iter1))
                    iteration_yes_SV.append(int(SV_iter1))

                # for time thread
                if int(Deadlock_info1) != -1:
                    time_thread_SE.append(round(float(SE_time1)/60, 2))
                    time_thread_SV.append(round(float(SV_time1)/60, 2))


            elif object == "Integrate-MS" or object == "Diffusion2d" or object == "Mandelbrot-MS" or object == "Sorting-MS" or object == "Kfray-MS":
                Deadlock_info1_1 = Deadlock_info1[:Deadlock_info1.find(" /")]
                Deadlock_info1_2 = Deadlock_info1[Deadlock_info1.find(" /") + 3:]
                SE_time1_1 = SE_time1[:SE_time1.find(" /")]
                SE_time1_2 = SE_time1[SE_time1.find(" /") + 3:]
                SV_time1_1 = SV_time1[:SV_time1.find(" /")]
                SV_time1_2 = SV_time1[SV_time1.find(" /") + 3:]
                SE_iter1_1 = SE_iter1[:SE_iter1.find(" /")]
                SE_iter1_2 = SE_iter1[SE_iter1.find(" /") + 3:]
                SV_iter1_1 = SV_iter1[:SV_iter1.find(" /")]
                SV_iter1_2 = SV_iter1[SV_iter1.find(" /") + 3:]


                if int(Deadlock_info1_1) == 0:
                    if SE_time1_1 != "to":
                        deadlockfree_SE += 1
                    if SV_time1_1 != "to":
                        deadlockfree_SV += 1
                    if SE_time1_1 == "to":
                        SE_time1_1 = "3600"
                    time_no_SE.append(float(SE_time1_1))
                    time_no_SV.append(float(SV_time1_1))
                    iteration_no_SE.append(int(SE_iter1_1))
                    iteration_no_SV.append(int(SV_iter1_1))
                elif int(Deadlock_info1_1) == 1:
                    if SE_time1_1 != "to":
                        deadlock_SE += 1
                    if SV_time1_1 != "to":
                        deadlock_SV += 1
                    if SE_time1_1 == "to":
                        SE_time1_1 = "3600"
                    time_yes_SE.append(float(SE_time1_1))
                    time_yes_SV.append(float(SV_time1_1))
                    iteration_yes_SE.append(int(SE_iter1_1))
                    iteration_yes_SV.append(int(SV_iter1_1))
                if int(Deadlock_info1_2) == 0:
                    if SE_time1_2 != "to":
                        deadlockfree_SE += 1
                    if SV_time1_2 != "to":
                        deadlockfree_SV += 1
                    if SE_time1_2 == "to":
                        SE_time1_2 = "3600"
                    time_no_SE.append(float(SE_time1_2))
                    time_no_SV.append(float(SV_time1_2))
                    iteration_no_SE.append(int(SE_iter1_2))
                    iteration_no_SV.append(int(SV_iter1_2))
                elif int(Deadlock_info1_2) == 1:
                    if SE_time1_2 != "to":
                        deadlock_SE += 1
                    if SV_time1_2 != "to":
                        deadlock_SV += 1
                    if SE_time1_2 == "to":
                        SE_time1_2 = "3600"
                    time_yes_SE.append(float(SE_time1_2))
                    time_yes_SV.append(float(SV_time1_2))
                    iteration_yes_SE.append(int(SE_iter1_2))
                    iteration_yes_SV.append(int(SV_iter1_2))


                # for time thread
                if int(Deadlock_info1_1) != -1:
                    time_thread_SE.append(round(float(SE_time1_1) / 60, 2))
                    time_thread_SV.append(round(float(SV_time1_1) / 60, 2))
                if int(Deadlock_info1_2) != -1:
                    time_thread_SE.append(round(float(SE_time1_2) / 60, 2))
                    time_thread_SV.append(round(float(SV_time1_2) / 60, 2))



            else:
                Deadlock_info1_1 = Deadlock_info1[:Deadlock_info1.find(" /")]
                Deadlock_info1_2 = Deadlock_info1[Deadlock_info1.find(" /") + 3: Deadlock_info1.rfind(" /")]
                Deadlock_info1_3 = Deadlock_info1[Deadlock_info1.rfind(" /") + 3:]
                SE_time1_1 = SE_time1[:SE_time1.find(" /")]
                SE_time1_2 = SE_time1[SE_time1.find(" /") + 3: SE_time1.rfind(" /")]
                SE_time1_3 = SE_time1[SE_time1.rfind(" /") + 3:]
                SV_time1_1 = SV_time1[:SV_time1.find(" /")]
                SV_time1_2 = SV_time1[SV_time1.find(" /") + 3: SV_time1.rfind(" /")]
                SV_time1_3 = SV_time1[SV_time1.rfind(" /") + 3:]
                SE_iter1_1 = SE_iter1[:SE_iter1.find(" /")]
                SE_iter1_2 = SE_iter1[SE_iter1.find(" /") + 3: SE_iter1.rfind(" /")]
                SE_iter1_3 = SE_iter1[SE_iter1.rfind(" /") + 3:]
                SV_iter1_1 = SV_iter1[:SV_iter1.find(" /")]
                SV_iter1_2 = SV_iter1[SV_iter1.find(" /") + 3: SV_iter1.rfind(" /")]
                SV_iter1_3 = SV_iter1[SV_iter1.rfind(" /") + 3:]

                if int(Deadlock_info1_1) == 0:
                    if SE_time1_1 != "to":
                        deadlockfree_SE += 1
                    if SV_time1_1 != "to":
                        deadlockfree_SV += 1
                    if SE_time1_1 == "to":
                        SE_time1_1 = "3600"
                    time_no_SE.append(float(SE_time1_1))
                    time_no_SV.append(float(SV_time1_1))
                    iteration_no_SE.append(int(SE_iter1_1))
                    iteration_no_SV.append(int(SV_iter1_1))
                elif int(Deadlock_info1_1) == 1:
                    if SE_time1_1 != "to":
                        deadlock_SE += 1
                    if SV_time1_1 != "to":
                        deadlock_SV += 1
                    if SE_time1_1 == "to":
                        SE_time1_1 = "3600"
                    time_yes_SE.append(float(SE_time1_1))
                    time_yes_SV.append(float(SV_time1_1))
                    iteration_yes_SE.append(int(SE_iter1_1))
                    iteration_yes_SV.append(int(SV_iter1_1))
                if int(Deadlock_info1_2) == 0:
                    if SE_time1_2 != "to":
                        deadlockfree_SE += 1
                    if SV_time1_2 != "to":
                        deadlockfree_SV += 1
                    if SE_time1_2 == "to":
                        SE_time1_2 = "3600"
                    time_no_SE.append(float(SE_time1_2))
                    time_no_SV.append(float(SV_time1_2))
                    iteration_no_SE.append(int(SE_iter1_2))
                    iteration_no_SV.append(int(SV_iter1_2))
                elif int(Deadlock_info1_2) == 1:
                    if SE_time1_2 != "to":
                        deadlock_SE += 1
                    if SV_time1_2 != "to":
                        deadlock_SV += 1
                    if SE_time1_2 == "to":
                        SE_time1_2 = "3600"
                    time_yes_SE.append(float(SE_time1_2))
                    time_yes_SV.append(float(SV_time1_2))
                    iteration_yes_SE.append(int(SE_iter1_2))
                    iteration_yes_SV.append(int(SV_iter1_2))
                if int(Deadlock_info1_3) == 0:
                    if SE_time1_3 != "to":
                        deadlockfree_SE += 1
                    if SV_time1_3 != "to":
                        deadlockfree_SV += 1
                    if SE_time1_3 == "to":
                        SE_time1_3 = "3600"
                    time_no_SE.append(float(SE_time1_3))
                    time_no_SV.append(float(SV_time1_3))
                    iteration_no_SE.append(int(SE_iter1_3))
                    iteration_no_SV.append(int(SV_iter1_3))
                elif int(Deadlock_info1_3) == 1:
                    if SE_time1_3 != "to":
                        deadlock_SE += 1
                    if SV_time1_3 != "to":
                        deadlock_SV += 1
                    if SE_time1_3 == "to":
                        SE_time1_3 = "3600"
                    time_yes_SE.append(float(SE_time1_3))
                    time_yes_SV.append(float(SV_time1_3))
                    iteration_yes_SE.append(int(SE_iter1_3))
                    iteration_yes_SV.append(int(SV_iter1_3))


                # for time thread
                if int(Deadlock_info1_1) != -1:
                    time_thread_SE.append(round(float(SE_time1_1) / 60, 2))
                    time_thread_SV.append(round(float(SV_time1_1) / 60, 2))
                if int(Deadlock_info1_2) != -1:
                    time_thread_SE.append(round(float(SE_time1_2) / 60, 2))
                    time_thread_SV.append(round(float(SV_time1_2) / 60, 2))
                if int(Deadlock_info1_3) != -1:
                    time_thread_SE.append(round(float(SE_time1_3) / 60, 2))
                    time_thread_SV.append(round(float(SV_time1_3) / 60, 2))

        time_deadlockfree_SE[object] = time_no_SE
        time_deadlockfree_SV[object] = time_no_SV
        time_deadlock_SE[object] = time_yes_SE
        time_deadlock_SV[object] = time_yes_SV
        iteration_deadlockfree_SE[object] = iteration_no_SE
        iteration_deadlockfree_SV[object] = iteration_no_SV
        iteration_deadlock_SE[object] = iteration_yes_SE
        iteration_deadlock_SV[object] = iteration_yes_SV







    time_deadlockfree_SE_sum = []
    time_deadlockfree_SV_sum = []
    time_deadlock_SE_sum = []
    time_deadlock_SV_sum = []
    iteration_deadlockfree_SE_sum = []
    iteration_deadlockfree_SV_sum = []
    iteration_deadlock_SE_sum = []
    iteration_deadlock_SV_sum = []

    for object in objects:
        # print(object)
        # print("--------------------------------\n")
        time_deadlockfree_SE_sum += time_deadlockfree_SE[object]
        time_deadlockfree_SV_sum += time_deadlockfree_SV[object]
        time_deadlock_SE_sum += time_deadlock_SE[object]
        time_deadlock_SV_sum += time_deadlock_SV[object]
        iteration_deadlockfree_SE_sum += iteration_deadlockfree_SE[object]
        iteration_deadlockfree_SV_sum += iteration_deadlockfree_SV[object]
        iteration_deadlock_SE_sum += iteration_deadlock_SE[object]
        iteration_deadlock_SV_sum += iteration_deadlock_SV[object]


    print("SE'time spend on deadlock-free: \n", sum(time_deadlockfree_SE_sum))
    print("num: ", len(time_deadlockfree_SE_sum))
    print("======================")
    print("SV'time spend on deadlock-free: \n", sum(time_deadlockfree_SV_sum))
    print("num: ", len(time_deadlockfree_SV_sum))
    print("======================")
    print("SE'time spend on deadlock: \n", sum(time_deadlock_SE_sum))
    print("num: ", len(time_deadlock_SE_sum))
    print("======================")
    print("SV'time spend on deadlock: \n", sum(time_deadlock_SV_sum))
    print("num: ", len(time_deadlock_SV_sum))
    print("======================")
    print("SE'iteration spend on deadlock-free: \n", sum(iteration_deadlockfree_SE_sum))
    print("num: ", len(iteration_deadlockfree_SE_sum))
    print("======================")
    print("SV'iteration spend on deadlock-free: \n", sum(iteration_deadlockfree_SV_sum))
    print("num: ", len(iteration_deadlockfree_SV_sum))
    print("======================")
    print("SE'iteration spend on deadlock: \n", sum(iteration_deadlock_SE_sum))
    print("num: ", len(iteration_deadlock_SE_sum))
    print("======================")
    print("SV'iteration spend on deadlock: \n", sum(iteration_deadlock_SV_sum))
    print("num: ", len(iteration_deadlock_SV_sum))
    print("======================")
    # print(Fraction(sum(time_deadlockfree_SV_sum)/sum(time_deadlockfree_SE_sum)).limit_denominator())
    print("time speed on deadlock-free: ", 1/(sum(time_deadlockfree_SV_sum)/sum(time_deadlockfree_SE_sum)))
    print("time speed on deadlock: ", 1/(sum(time_deadlock_SV_sum) / sum(time_deadlock_SE_sum)))
    print("iteration speed on deadlock-free: ", 1/(sum(iteration_deadlockfree_SV_sum) / sum(iteration_deadlockfree_SE_sum)))
    print("iteration speed on deadlock: ", 1/(sum(iteration_deadlock_SV_sum) / sum(iteration_deadlock_SE_sum)))
    print("======================")
    print("SE")
    print("\t total: ", deadlock_SE + deadlockfree_SE, round((deadlock_SE + deadlockfree_SE)/111, 2))
    print("\t deadlock: ", deadlock_SE, "(", round(deadlock_SE / 111, 2), ")")
    print("\t deadlock_free: ", deadlockfree_SE, "(", round(deadlockfree_SE / 111, 2), ")")
    print("SV")
    print("\t total: ", deadlock_SV + deadlockfree_SV, round((deadlock_SV + deadlockfree_SV) / 111, 2))
    print("\t deadlock: ", deadlock_SV, "(", round(deadlock_SV / 111, 2), ")")
    print("\t deadlock_free: ", deadlockfree_SV, "(", round(deadlockfree_SV / 111, 2), ")")
    print("======================")


    for item in range(5, 65, 5):
        print("time: ", item)
        num_SE=0
        num_SV=0
        for item_SE in time_thread_SE:
            if item_SE < item:
                num_SE += 1
        for item_SV in time_thread_SV:
            if item_SV < item:
                num_SV += 1
        print("\t SE: ", num_SE)
        print("\t SV: ", num_SV)
        print("-----------------")