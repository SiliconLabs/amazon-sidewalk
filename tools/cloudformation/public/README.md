# Sidewalk CF template know-how

Sidewalk CF is for deploying related stack into AWS Cloud easily and prevent resource loss in case of any changes in the cloud.
The template file itself can be found in the sidewalk stash repository, which is used for every-day develop processes, but
it is not used for updating the stack, but storing the newest changes as a backup. Another repository in DevOps project is for
deploying and maintaining sidewalk-stack.

## Important

If the template file has to be modified, a PR has to be raised in the repo mentioned above. The repo can be found at:
* https://stash.silabs.com/projects/DEVOPS/repos/sidewalk/browse

In normal case, the branch must be opened from master, and if the CI run is successful, the stack is deployed into cloud, and the 
branch can be merged into master.  

## Good-to-know

If there are any needs regarding to AWS stack, and the template file has changed in order to introduce the changes, the CI 
will create/modify/remove anything that is needed, because it has full access to the cloud. So, with other words, the changes 
can be placed without asking help from devops team.