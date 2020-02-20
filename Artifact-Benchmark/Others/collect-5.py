#!/usr/bin/env python
#-*- coding:utf-8 -*-

import xlwt

object_file_map = {"DTG":"DTG", "Matmat-MS":"Matmat-MS", "Integrate":"Integrate",
                   "Integrate-MS":"Integrate-MS", "Diffusion2d":"Diffusion2d",
           "Gauss_elim":"Gauss_elim", "Heat":"Heat", "Mandelbrot":"Mandelbrot",
                   "Mandelbrot-MS":"Mandelbrot-MS", "Sorting-MS":"Sorting-MS",
           "Image_mani":"Image_mani", "DepSolver":"DepSolver", "Kfray":"Kfray",
                   "Kfray-MS":"Kfray-MS", "ClustalW":"Clustalw"}


def locate_key(object, key_str):
    f = open(file_root+object_file_map[object]+".txt")
    for line in f:
        if key_str in line:
            f.close()
            return line
    return False



if __name__ == "__main__":
    file_root = "./5_min/"


    objects = ["DTG", "Matmat-MS", "Integrate", "Integrate-MS", "Diffusion2d",
               "Gauss_elim", "Heat", "Mandelbrot", "Mandelbrot-MS", "Sorting-MS",
               "Image_mani", "DepSolver", "Kfray", "Kfray-MS", "ClustalW"]
    processes = {"DTG":[5],
                 "Matmat-MS":[4],
                 "Integrate":[6,8,10],
                 "Integrate-MS":[4,6],
                 "Diffusion2d":[4,6],
                 "Gauss_elim":[6,8,10],
                 "Heat":[6,8,10],
                 "Mandelbrot":[6,8,10],
                 "Mandelbrot-MS":[4,6],
                 "Sorting-MS":[4,6],
                 "Image_mani":[6,8,10],
                 "DepSolver":[6,8,10],
                 "Kfray":[6,8,10],
                "Kfray-MS":[4,6],
                 "ClustalW":[6,8,10]}
    proc_num = {"DTG": "(5)",
                 "Matmat-MS": "(4)",
                 "Integrate": "(6 / 8 / 10)",
                 "Integrate-MS": "(4 / 6)",
                 "Diffusion2d": "(4 / 6)",
                 "Gauss_elim": "(6 / 8 / 10)",
                 "Heat": "(6 / 8 / 10)",
                 "Mandelbrot": "(6 / 8 / 10)",
                 "Mandelbrot-MS": "(4 / 6)",
                 "Sorting-MS": "(4 / 6)",
                 "Image_mani": "(6 / 8 / 10)",
                 "DepSolver": "(6 / 8 / 10)",
                 "Kfray": "(6 / 8 / 10)",
                 "Kfray-MS": "(4 / 6)",
                 "ClustalW": "(6 / 8 / 10)"}
    mutants = {"DTG":[0, 1, 2, 3, 4, 5],
               "Matmat-MS":[0],
               "Integrate":[0,1,2],
               "Integrate-MS":[0],
               "Diffusion2d":[0, 1, 2, 3, 4, 5],
               "Gauss_elim":[0,1],
               "Heat":[0, 1, 2, 3, 4, 5],
               "Mandelbrot":[0,1,2,3],
               "Mandelbrot-MS":[0],
               "Sorting-MS":[0],
               "Image_mani":[0,1],
               "DepSolver":[0],
               "Kfray":[0,1,2,3],
               "Kfray-MS":[0],
               "ClustalW":[0, 1, 2, 3, 4, 5]}

    workbook = xlwt.Workbook()
    worksheet = workbook.add_sheet("5-min")
    worksheet.write(0, 0, 'Program(#procs)')
    worksheet.write(0, 1, 'T')
    worksheet.write(0, 2, 'Deadlock')
    worksheet.write(0, 3, 'Symbolic Execution')
    worksheet.write(0, 4, 'MPI-SV')
    worksheet.write(0, 5, 'SE-time')
    worksheet.write(0, 6, 'SV-time')
    write_line=0
    for object in objects:
        mutant=mutants[object]
        process=processes[object]


        for mutant_i in mutant:
            worksheet.write(write_line + 1, 0, object + " " + str(proc_num[object]))
            worksheet.write(write_line + 1, 1, 'm' + str(mutant_i))
            iteration_SE = ""
            iteration_SV = ""
            deadlock = ""
            time_SE = ""
            time_SV = ""
            for process_i in process:

                # key_find
                deadlock_SE_flag_command = "mut" + str(mutant_i) + "_process" + str(process_i) + "_opt0.log   ------------>    Deadlock:"
                deadlock_SV_flag_command = "mut" + str(mutant_i) + "_process" + str(process_i) + "_opt1.log   ------------>    Deadlock:"
                iteration_SE_command = "mut" + str(mutant_i) + "_process" + str(process_i) + "_opt0.log   ------------>    MPI-SV:"
                iteration_SV_command = "mut" + str(mutant_i) + "_process" + str(process_i) + "_opt1.log   ------------>    MPI-SV:"
                time_SE_command = "mut" + str(mutant_i) + "_process" + str(process_i) + "_opt0.log   ------------>    TIME:"
                time_SV_command = "mut" + str(mutant_i) + "_process" + str(process_i) + "_opt1.log   ------------>    TIME:"


                if locate_key(object, deadlock_SE_flag_command) != False and locate_key(object, deadlock_SV_flag_command) != False:
                    # line_find
                    line_deadlock_SE = locate_key(object, deadlock_SE_flag_command)
                    line_deadlock_SV = locate_key(object, deadlock_SV_flag_command)
                    line_iteration_SE = locate_key(object, iteration_SE_command)
                    line_iteration_SV = locate_key(object, iteration_SV_command)
                    line_time_SE = locate_key(object, time_SE_command)
                    line_time_SV = locate_key(object, time_SV_command)

                    # judgement
                    if "unknown" in line_deadlock_SE and "unknown" in line_deadlock_SV:
                        deadlock += "-1 / "
                    elif "yes" in line_deadlock_SE or "yes" in line_deadlock_SV:
                        deadlock += "1 / "
                    elif "no" in line_deadlock_SE or "no" in line_deadlock_SV:
                        deadlock += "0 / "
                    iteration_SE += line_iteration_SE[line_iteration_SE.find("totally")+8:line_iteration_SE.find("iterations")-1] + " / "
                    iteration_SV += line_iteration_SV[line_iteration_SV.find("totally") + 8:line_iteration_SV.find("iterations") - 1] + " / "

                    if "HaltTimer" in line_time_SE:
                        time_SE += "TO / "
                    else:
                        time_SE += line_time_SE[line_time_SE.find("real") + 5:-1] + " / "
                    if "HaltTimer" in line_time_SV:
                        time_SV += "TO / "
                    else:
                        time_SV += line_time_SV[line_time_SV.find("real") + 5:-1] + " / "
                elif locate_key(object, deadlock_SE_flag_command) == False and locate_key(object, deadlock_SV_flag_command) != False:
                    # line_find
                    line_deadlock_SV = locate_key(object, deadlock_SV_flag_command)
                    line_iteration_SV = locate_key(object, iteration_SV_command)
                    line_time_SV = locate_key(object, time_SV_command)

                    # judgement
                    if "yes" in line_deadlock_SV:
                        deadlock += "1 / "
                    elif "no" in line_deadlock_SV:
                        deadlock += "0 / "
                    iteration_SV += line_iteration_SV[line_iteration_SV.find("totally") + 8:line_iteration_SV.find(
                        "iterations") - 1] + " / "

                    time_SV += line_time_SV[line_time_SV.find("real") + 5:-1] + " / "
                    iteration_SE += "_ / "
                    time_SE += "_ / "
                elif locate_key(object, deadlock_SE_flag_command) != False and locate_key(object, deadlock_SV_flag_command) == False:
                    # line_find
                    line_deadlock_SE = locate_key(object, deadlock_SE_flag_command)
                    line_iteration_SE = locate_key(object, iteration_SE_command)
                    line_time_SE = locate_key(object, time_SE_command)

                    # judgement
                    if "yes" in line_deadlock_SE:
                        deadlock += "1 / "
                    elif "no" in line_deadlock_SE:
                        deadlock += "0 / "
                    iteration_SE += line_iteration_SE[line_iteration_SE.find("totally") + 8:line_iteration_SE.find(
                        "iterations") - 1] + " / "

                    time_SE += line_time_SE[line_time_SE.find("real") + 5:-1] + " / "
                    iteration_SV += "_ / "
                    time_SV += "_ / "
                else:
                    deadlock += "_ / "
                    iteration_SE += "_ / "
                    iteration_SV += "_ / "
                    time_SE += "_ / "
                    time_SV += "_ / "

            worksheet.write(write_line + 1, 2, deadlock[0:-3])
            worksheet.write(write_line + 1, 3, iteration_SE[0:-3])
            worksheet.write(write_line + 1, 4, iteration_SV[0:-3])
            worksheet.write(write_line + 1, 5, time_SE[0:-3])
            worksheet.write(write_line + 1, 6, time_SV[0:-3])
            write_line += 1
        write_line += 1

    workbook.save('5-min.xls')