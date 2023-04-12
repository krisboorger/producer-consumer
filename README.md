# Task
This task is a much more complex version of the classic producer-consumer problem.
This diagram shows the flow of items between the processes:
```
 Consumers -> Workers -> Master -> Delivery
                ^           |
                |           |
                -------------
```
Consumers order items to be crafted, Workers craft them, Master checks their quality and returns some items back to the Workers, and Delivery ensures their, well, delivery.
Since there is some flow of items from Master back to the Workers, this inevitably leads to deadlocks, which are solved by introducing one more process called the Supervisor.
It starts its intervention when the youngest item in Workers' queue is older than Z (a given constant) and ends it after sorting out the problem to considerable degree.

Consumers and items come with different priorities which have to be taken care of (this is marked by different colors in cli output).
