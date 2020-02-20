#!/usr/bin/env python
#-*- coding:utf-8 -*-


import xlrd
import xlwt
from math import ceil

line_map = {"DTG":[1,2,3,4,5,6],
            "Matmat-MS":[8],
            "Integrate":[10,11,12],
            "Integrate-MS":[14],
            "Diffusion2d":[16,17,18,19,20,21],
            "Gauss_elim":[23,24],
            "Heat":[26,27,28,29,30,31],
            "Mandelbrot":[33,34,35,36],
            "Mandelbrot-MS":[38],
            "Sorting-MS":[40],
            "Image_mani":[42,43],
            "DepSolver":[45],
            "Kfray":[47,48,49,50],
            "Kfray-MS":[52],
            "ClustalW":[54,55,56,57,58,59]}

objects = ["DTG", "Matmat-MS", "Integrate", "Integrate-MS", "Diffusion2d",
           "Gauss_elim", "Heat", "Mandelbrot", "Mandelbrot-MS", "Sorting-MS",
           "Image_mani", "DepSolver", "Kfray", "Kfray-MS", "ClustalW"]


if __name__ == "__main__":

    file1 = xlrd.open_workbook(r'./epoch1.xls')
    table1 = file1.sheet_by_index(0)
    file2 = xlrd.open_workbook(r'./epoch2.xls')
    table2 = file2.sheet_by_index(0)
    file3 = xlrd.open_workbook(r'./epoch3.xls')
    table3 = file3.sheet_by_index(0)

    workbook = xlwt.Workbook()
    worksheet = workbook.add_sheet("average")
    worksheet.write(0, 0, 'Program(#procs)')
    worksheet.write(0, 1, 'T')
    worksheet.write(0, 2, 'Deadlock')
    worksheet.write(0, 3, 'Symbolic Execution')
    worksheet.write(0, 4, 'MPI-SV')
    worksheet.write(0, 5, 'SE-time')
    worksheet.write(0, 6, 'SV-time')


    for object in objects:
        for line in line_map[object]:
            worksheet.write(line, 0, table1.cell(line,0).value)
            worksheet.write(line, 1, table1.cell(line, 1).value)

            Deadlock_info1 = table1.cell(line, 2).value
            Deadlock_info2 = table2.cell(line, 2).value
            Deadlock_info3 = table3.cell(line, 2).value

            SE_iter1 = table1.cell(line, 3).value
            SE_iter2 = table2.cell(line, 3).value
            SE_iter3 = table3.cell(line, 3).value

            SV_iter1 = table1.cell(line, 4).value
            SV_iter2 = table2.cell(line, 4).value
            SV_iter3 = table3.cell(line, 4).value

            SE_time1 = table1.cell(line, 5).value
            SE_time2 = table2.cell(line, 5).value
            SE_time3 = table3.cell(line, 5).value

            SV_time1 = table1.cell(line, 6).value
            SV_time2 = table2.cell(line, 6).value
            SV_time3 = table3.cell(line, 6).value

            if object == "DTG" or object == "Matmat-MS":
                if Deadlock_info1==Deadlock_info2 and Deadlock_info1==Deadlock_info3:
                    Deadlock_info = Deadlock_info1
                else:
                    print(object, line)
                    exit()
                SE_iter = (int(SE_iter1) + int(SE_iter2) + int(SE_iter3)) / 3
                SV_iter = (int(SV_iter1) + int(SV_iter2) + int(SV_iter3)) / 3
                SE_time = round((float(SE_time1) + float(SE_time2) + float(SE_time3)) / 3, 2)
                SV_time = round((float(SV_time1) + float(SV_time2) + float(SV_time3)) / 3, 2)
                worksheet.write(line, 2, Deadlock_info)
                worksheet.write(line, 3, ceil(SE_iter))
                worksheet.write(line, 4, ceil(SV_iter))
                worksheet.write(line, 5, SE_time)
                worksheet.write(line, 6, SV_time)

            elif object == "Integrate-MS" or object == "Diffusion2d" or object == "Mandelbrot-MS" or object == "Sorting-MS" or object == "Kfray-MS":
                Deadlock_info1_1 = Deadlock_info1[:Deadlock_info1.find(" /")]
                Deadlock_info1_2 = Deadlock_info1[Deadlock_info1.find(" /") + 3:]
                Deadlock_info2_1 = Deadlock_info2[:Deadlock_info2.find(" /")]
                Deadlock_info2_2 = Deadlock_info2[Deadlock_info2.find(" /") + 3:]
                Deadlock_info3_1 = Deadlock_info3[:Deadlock_info3.find(" /")]
                Deadlock_info3_2 = Deadlock_info3[Deadlock_info3.find(" /") + 3:]
                if Deadlock_info1_1 == Deadlock_info2_1 and Deadlock_info1_1 == Deadlock_info3_1:
                    Deadlock_info_1 = Deadlock_info1_1
                else:
                    print(object, line)
                    print("deadlock inconsistence")
                    exit()
                if Deadlock_info1_2 == Deadlock_info2_2 and Deadlock_info1_2 == Deadlock_info3_2:
                    Deadlock_info_2 = Deadlock_info1_2
                else:
                    print(object, line)
                    print("deadlock inconsistence")
                    exit()

                SE_iter1_1 = SE_iter1[:SE_iter1.find(" /")]
                SE_iter1_2 = SE_iter1[SE_iter1.find(" /") + 3:]
                SE_iter2_1 = SE_iter2[:SE_iter2.find(" /")]
                SE_iter2_2 = SE_iter2[SE_iter2.find(" /") + 3:]
                SE_iter3_1 = SE_iter3[:SE_iter3.find(" /")]
                SE_iter3_2 = SE_iter3[SE_iter3.find(" /") + 3:]
                SE_iter_1 = (int(SE_iter1_1) + int(SE_iter2_1) + int(SE_iter3_1)) / 3
                SE_iter_2 = (int(SE_iter1_2) + int(SE_iter2_2) + int(SE_iter3_2)) / 3

                SV_iter1_1 = SV_iter1[:SV_iter1.find(" /")]
                SV_iter1_2 = SV_iter1[SV_iter1.find(" /") + 3:]
                SV_iter2_1 = SV_iter2[:SV_iter2.find(" /")]
                SV_iter2_2 = SV_iter2[SV_iter2.find(" /") + 3:]
                SV_iter3_1 = SV_iter3[:SV_iter3.find(" /")]
                SV_iter3_2 = SV_iter3[SV_iter3.find(" /") + 3:]
                SV_iter_1 = (int(SV_iter1_1) + int(SV_iter2_1) + int(SV_iter3_1)) / 3
                SV_iter_2 = (int(SV_iter1_2) + int(SV_iter2_2) + int(SV_iter3_2)) / 3

                SE_time1_1 = SE_time1[:SE_time1.find(" /")]
                SE_time1_2 = SE_time1[SE_time1.find(" /") + 3:]
                SE_time2_1 = SE_time2[:SE_time2.find(" /")]
                SE_time2_2 = SE_time2[SE_time2.find(" /") + 3:]
                SE_time3_1 = SE_time3[:SE_time3.find(" /")]
                SE_time3_2 = SE_time3[SE_time3.find(" /") + 3:]
                if SE_time1_1 != "to" and SE_time2_1 != "to" and SE_time3_1 != "to":
                    SE_time_1 = str(round((float(SE_time1_1) + float(SE_time2_1) + float(SE_time3_1)) / 3, 2))
                else:
                    SE_time_1 = "to"
                if SE_time1_2 != "to" and SE_time2_2 != "to" and SE_time3_2 != "to":
                    SE_time_2 = str(round((float(SE_time1_2) + float(SE_time2_2) + float(SE_time3_2)) / 3, 2))
                else:
                    SE_time_2 = "to"

                SV_time1_1 = SV_time1[:SV_time1.find(" /")]
                SV_time1_2 = SV_time1[SV_time1.find(" /") + 3:]
                SV_time2_1 = SV_time2[:SV_time2.find(" /")]
                SV_time2_2 = SV_time2[SV_time2.find(" /") + 3:]
                SV_time3_1 = SV_time3[:SV_time3.find(" /")]
                SV_time3_2 = SV_time3[SV_time3.find(" /") + 3:]
                if SV_time1_1 != "to" and SV_time2_1 != "to" and SV_time3_1 != "to":
                    SV_time_1 = str(round((float(SV_time1_1) + float(SV_time2_1) + float(SV_time3_1)) / 3, 2))
                else:
                    SV_time_1 = "to"
                if SV_time1_2 != "to" and SV_time2_2 != "to" and SV_time3_2 != "to":
                    SV_time_2 = str(round((float(SV_time1_2) + float(SV_time2_2) + float(SV_time3_2)) / 3, 2))
                else:
                    SV_time_2 = "to"

                worksheet.write(line, 2, Deadlock_info_1 + " / " + Deadlock_info_2)
                worksheet.write(line, 3, str(ceil(SE_iter_1)) + " / " + str(ceil(SE_iter_2)))
                worksheet.write(line, 4, str(ceil(SV_iter_1)) + " / " + str(ceil(SV_iter_2)))
                worksheet.write(line, 5, SE_time_1 + " / " + SE_time_2)
                worksheet.write(line, 6, SV_time_1 + " / " + SV_time_2)

            else:
                Deadlock_info1_1 = Deadlock_info1[:Deadlock_info1.find(" /")]
                Deadlock_info1_2 = Deadlock_info1[Deadlock_info1.find(" /") + 3: Deadlock_info1.rfind(" /")]
                Deadlock_info1_3 = Deadlock_info1[Deadlock_info1.rfind(" /") + 3:]
                Deadlock_info2_1 = Deadlock_info2[:Deadlock_info2.find(" /")]
                Deadlock_info2_2 = Deadlock_info2[Deadlock_info2.find(" /") + 3: Deadlock_info2.rfind(" /")]
                Deadlock_info2_3 = Deadlock_info2[Deadlock_info2.rfind(" /") + 3:]
                Deadlock_info3_1 = Deadlock_info3[:Deadlock_info3.find(" /")]
                Deadlock_info3_2 = Deadlock_info3[Deadlock_info3.find(" /") + 3: Deadlock_info3.rfind(" /")]
                Deadlock_info3_3 = Deadlock_info3[Deadlock_info3.rfind(" /") + 3:]
                if Deadlock_info1_1 == Deadlock_info2_1 and Deadlock_info1_1 == Deadlock_info3_1:
                    Deadlock_info_1 = Deadlock_info1_1
                else:
                    print(object, line)
                    print("deadlock inconsistence")
                    exit()
                if Deadlock_info1_2 == Deadlock_info2_2 and Deadlock_info1_2 == Deadlock_info3_2:
                    Deadlock_info_2 = Deadlock_info1_2
                else:
                    print(object, line)
                    print("deadlock inconsistence")
                    exit()
                if Deadlock_info1_3 == Deadlock_info2_3 and Deadlock_info1_3 == Deadlock_info3_3:
                    Deadlock_info_3 = Deadlock_info1_3
                else:
                    print(object, line)
                    print("deadlock inconsistence")
                    exit()

                SE_iter1_1 = SE_iter1[:SE_iter1.find(" /")]
                SE_iter1_2 = SE_iter1[SE_iter1.find(" /") + 3: SE_iter1.rfind(" /")]
                SE_iter1_3 = SE_iter1[SE_iter1.rfind(" /") + 3:]
                SE_iter2_1 = SE_iter2[:SE_iter2.find(" /")]
                SE_iter2_2 = SE_iter2[SE_iter2.find(" /") + 3: SE_iter2.rfind(" /")]
                SE_iter2_3 = SE_iter2[SE_iter2.rfind(" /") + 3:]
                SE_iter3_1 = SE_iter3[:SE_iter3.find(" /")]
                SE_iter3_2 = SE_iter3[SE_iter3.find(" /") + 3: SE_iter3.rfind(" /")]
                SE_iter3_3 = SE_iter3[SE_iter3.rfind(" /") + 3:]
                SE_iter_1 = (int(SE_iter1_1) + int(SE_iter2_1) + int(SE_iter3_1)) / 3
                SE_iter_2 = (int(SE_iter1_2) + int(SE_iter2_2) + int(SE_iter3_2)) / 3
                SE_iter_3 = (int(SE_iter1_3) + int(SE_iter2_3) + int(SE_iter3_3)) / 3

                SV_iter1_1 = SV_iter1[:SV_iter1.find(" /")]
                SV_iter1_2 = SV_iter1[SV_iter1.find(" /") + 3: SV_iter1.rfind(" /")]
                SV_iter1_3 = SV_iter1[SV_iter1.rfind(" /") + 3:]
                SV_iter2_1 = SV_iter2[:SV_iter2.find(" /")]
                SV_iter2_2 = SV_iter2[SV_iter2.find(" /") + 3: SV_iter2.rfind(" /")]
                SV_iter2_3 = SV_iter2[SV_iter2.rfind(" /") + 3:]
                SV_iter3_1 = SV_iter3[:SV_iter3.find(" /")]
                SV_iter3_2 = SV_iter3[SV_iter3.find(" /") + 3: SV_iter3.rfind(" /")]
                SV_iter3_3 = SV_iter3[SV_iter3.rfind(" /") + 3:]
                SV_iter_1 = (int(SV_iter1_1) + int(SV_iter2_1) + int(SV_iter3_1)) / 3
                SV_iter_2 = (int(SV_iter1_2) + int(SV_iter2_2) + int(SV_iter3_2)) / 3
                SV_iter_3 = (int(SV_iter1_3) + int(SV_iter2_3) + int(SV_iter3_3)) / 3

                SE_time1_1 = SE_time1[:SE_time1.find(" /")]
                SE_time1_2 = SE_time1[SE_time1.find(" /") + 3: SE_time1.rfind(" /")]
                SE_time1_3 = SE_time1[SE_time1.rfind(" /") + 3:]
                SE_time2_1 = SE_time2[:SE_time2.find(" /")]
                SE_time2_2 = SE_time2[SE_time2.find(" /") + 3: SE_time2.rfind(" /")]
                SE_time2_3 = SE_time2[SE_time2.rfind(" /") + 3:]
                SE_time3_1 = SE_time3[:SE_time3.find(" /")]
                SE_time3_2 = SE_time3[SE_time3.find(" /") + 3: SE_time3.rfind(" /")]
                SE_time3_3 = SE_time3[SE_time3.rfind(" /") + 3:]
                if SE_time1_1 != "to" and SE_time2_1 != "to" and SE_time3_1 != "to":
                    SE_time_1 = str(round((float(SE_time1_1) + float(SE_time2_1) + float(SE_time3_1)) / 3, 2))
                else:
                    SE_time_1 = "to"
                if SE_time1_2 != "to" and SE_time2_2 != "to" and SE_time3_2 != "to":
                    SE_time_2 = str(round((float(SE_time1_2) + float(SE_time2_2) + float(SE_time3_2)) / 3, 2))
                else:
                    SE_time_2 = "to"
                if SE_time1_3 != "to" and SE_time2_3 != "to" and SE_time3_3 != "to":
                    SE_time_3 = str(round((float(SE_time1_3) + float(SE_time2_3) + float(SE_time3_3)) / 3, 2))
                else:
                    SE_time_3 = "to"

                SV_time1_1 = SV_time1[:SV_time1.find(" /")]
                SV_time1_2 = SV_time1[SV_time1.find(" /") + 3: SV_time1.rfind(" /")]
                SV_time1_3 = SV_time1[SV_time1.rfind(" /") + 3:]
                SV_time2_1 = SV_time2[:SV_time2.find(" /")]
                SV_time2_2 = SV_time2[SV_time2.find(" /") + 3: SV_time2.rfind(" /")]
                SV_time2_3 = SV_time2[SV_time2.rfind(" /") + 3:]
                SV_time3_1 = SV_time3[:SV_time3.find(" /")]
                SV_time3_2 = SV_time3[SV_time3.find(" /") + 3: SV_time3.rfind(" /")]
                SV_time3_3 = SV_time3[SV_time3.rfind(" /") + 3:]
                if SV_time1_1 != "to" and SV_time2_1 != "to" and SV_time3_1 != "to":
                    SV_time_1 = str(round((float(SV_time1_1) + float(SV_time2_1) + float(SV_time3_1)) / 3, 2))
                else:
                    SV_time_1 = "to"
                if SV_time1_2 != "to" and SV_time2_2 != "to" and SV_time3_2 != "to":
                    SV_time_2 = str(round((float(SV_time1_2) + float(SV_time2_2) + float(SV_time3_2)) / 3, 2))
                else:
                    SV_time_2 = "to"
                if SV_time1_3 != "to" and SV_time2_3 != "to" and SV_time3_3 != "to":
                    SV_time_3 = str(round((float(SV_time1_3) + float(SV_time2_3) + float(SV_time3_3)) / 3, 2))
                else:
                    SV_time_3 = "to"

                worksheet.write(line, 2, Deadlock_info_1 + " / " + Deadlock_info_2 + " / " + Deadlock_info_3)
                worksheet.write(line, 3, str(ceil(SE_iter_1)) + " / " + str(ceil(SE_iter_2)) + " / " + str(ceil(SE_iter_3)))
                worksheet.write(line, 4, str(ceil(SV_iter_1)) + " / " + str(ceil(SV_iter_2)) + " / " + str(ceil(SV_iter_3)))
                worksheet.write(line, 5, SE_time_1 + " / " + SE_time_2 + " / " + SE_time_3)
                worksheet.write(line, 6, SV_time_1 + " / " + SV_time_2 + " / " + SV_time_3)

    workbook.save('average.xls')