import goldenrecord

#filepath = "../data/mitdwh/"
cname = "cluster_id"
#outfilepath = "./mitdwh_updated.csv"

#input_file = filepath
#output_file = outfilepath

def main(filepath, outfilepath, preput=""):
  consolidation = goldenrecord.Consolidation(filepath, cname)
  num_of_tables = consolidation.number_of_tables
  for i in range(num_of_tables):
    str_noc = consolidation.TryNextTable(i)
    for col_id in range(int(str_noc)):

      if col_id == consolidation.cluster_id_col:
        continue

      message1 = consolidation.ProfileColumn(i, col_id)
      print(message1)

      skip, preput = (preput[0:1], preput[1:]) if len(preput) > 0 else (input(), "")
      if skip != "1" and skip != "2" and skip != "3" and skip != "4" and skip != "5":
        continue

      message2 = consolidation.TryNextColumn(i, col_id, skip)
      print(message2)

      applied_group_num = 0

      var = 1
      while var == 1:
        message3 = consolidation.ShowNextCluster()
        strs = message3.split('\t', 2)
        max_group_id = int(strs[0])
        max_count = int(strs[1])
        print(strs[2])
        if max_count == 0:
          print("Done with current column\n")
          break

        choice, preput = (preput[0:1], preput[1:]) if len(preput) > 0 else (input(), "")
        if choice == "4":
          print("Done with current column\n")
          break

        message4 = consolidation.ApplyCluster(i, col_id, applied_group_num, max_group_id, choice)
        applied_group_num = applied_group_num + 1
        print(message4)

    consolidation.MaterializeTable(i, outfilepath)
    print("Successfully Exit!\n")
    break

#consolidation.ConsolidationGo()

