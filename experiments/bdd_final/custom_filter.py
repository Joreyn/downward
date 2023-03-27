from collections import defaultdict


class CustomFilter:

    def __init__(self):
        self.time_diff_list = defaultdict(list) #(domain,problem,algo,symbolic) ->list(float)
        self.time_diff = defaultdict(float) #(domain,problem,algo,symbolic) ->float
        self.time_diff_bdd_exp = defaultdict(float) #(domain,problem,algo,symbolic) ->float

        self.mem_diff_list = defaultdict(list) #(domain,problem,algo,symbolic) ->list(float)
        self.mem_diff = defaultdict(int) #(domain,problem,algo,symbolic) ->int
        self.mem_diff_bdd_exp = defaultdict(int) #(domain,problem,algo,symbolic) ->int

    def _get_task(self, run):
        return (run["domain"], run["problem"], run["algorithm"][:-3], run["algorithm"].endswith("bdd"))

    def _get_other_task(self, run):
        return (run["domain"], run["problem"], run["algorithm"][:-3], not run["algorithm"].endswith("bdd"))

    def compute_timediff(self, run):
        self.time_diff_list[self._get_task(run)] = [x-y for x, y in zip(run.get("PDB_finished_time"), run.get("PDB_create_time"))]
        self.time_diff[self._get_task(run)] = sum(self.time_diff_list[self._get_task(run)])
        return True

    def compute_timediff_bdd_exp(self, run):
        temp=[x-y for x,y in zip(self.time_diff_list[self._get_task(run)], self.time_diff_list[self._get_other_task(run)])]
        self.time_diff_bdd_exp[self._get_task(run)]=sum(temp)
        return True

    def add_timediff(self, run):
        run["time_diff"]=self.time_diff[self._get_task(run)]
        run["time_diff_bdd_exp"]=self.time_diff_bdd_exp[self._get_task(run)]
        return run

    def compute_memdiff(self, run):
        self.mem_diff_list[self._get_task(run)] = [x-y for x, y in zip(run.get("PDB_after_memory"), run.get("PDB_before_memory"))]
        self.mem_diff[self._get_task(run)] = sum(self.mem_diff_list[self._get_task(run)])
        return True

    def compute_memdiff_bdd_exp(self, run):
        self.mem_diff_bdd_exp[self._get_task(run)]=sum([x-y for x,y in zip(self.mem_diff_list[self._get_task(run)], self.mem_diff_list[self._get_other_task(run)])])
        return True

    def add_memdiff(self, run):
        run["mem_diff"]=self.mem_diff[self._get_task(run)]
        run["mem_diff_bdd_exp"]=self.mem_diff_bdd_exp[self._get_task(run)]
        return run

