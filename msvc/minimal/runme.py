import minimal

m1 = minimal.minimal()
m1.print_num_instances()


m2 = minimal.minimal()
m1.print_num_instances()

m3 = minimal.minimal()
m1.print_num_instances()

del m2
m1.print_num_instances()
