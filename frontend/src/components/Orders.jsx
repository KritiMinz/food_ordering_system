import { useEffect, useState } from "react"

const statusSteps = [
  "PLACED",
  "CONFIRMED",
  "PREPARING",
  "READY",
  "OUT_FOR_DELIVERY",
  "DELIVERED",
]

function Orders({ currentUser }) {
  const [orders, setOrders] = useState([])
  const [loading, setLoading] = useState(true)
  const [error, setError] = useState("")

const loadOrders = async () => {
  if (!currentUser) {
    setOrders([])
    setLoading(false)
    return
  }

  try {
    setLoading(true)
    setError("")

    const token = localStorage.getItem("authToken")

    if (!token) {
      throw new Error("Authentication required. Please login again.")
    }

    const response = await fetch(
      `http://localhost:18080/api/orders?userId=${currentUser.id}`,
      {
        method: "GET",
        headers: {
          "Content-Type": "application/json",
          Authorization: `Bearer ${token}`,
        },
      }
    )

    const data = await response.json()

    if (!response.ok) {
      throw new Error(
        data.message || "Failed to load orders"
      )
    }

    // Backend returns:
    // {
    //   "orders": [...],
    //   "status": "success"
    // }

    setOrders(data.orders || [])
  } catch (error) {
    console.error("Failed to load orders:", error)
    setError(error.message)
    setOrders([])
  } finally {
    setLoading(false)
  }
}

  const handleCancelOrder = async (orderId) => {
    const confirmed = window.confirm(
      "Are you sure you want to cancel this order?"
    )

    if (!confirmed) {
      return
    }

    try {
      const token = localStorage.getItem("authToken")

      if (!token) {
        throw new Error(
          "Authentication required. Please login again."
        )
      }

      const response = await fetch(
        `http://localhost:18080/api/orders/${orderId}/cancel`,
        {
          method: "PUT",
          headers: {
            "Content-Type": "application/json",
            Authorization: `Bearer ${token}`,
          },
        }
      )

      const data = await response.json()

      if (!response.ok) {
        throw new Error(
          data.message || "Failed to cancel order"
        )
      }

      await loadOrders()

      alert("Order cancelled successfully.")
    } catch (error) {
      console.error("Failed to cancel order:", error)
      alert(error.message)
    }
  }

  useEffect(() => {
    loadOrders()
  }, [currentUser])

  if (!currentUser) {
    return (
      <section className="max-w-4xl mx-auto px-6 pb-20">
        <div className="bg-white rounded-2xl shadow-md p-10 text-center">
          <div className="text-5xl mb-4">
            🔐
          </div>

          <h2 className="text-2xl font-bold text-gray-800">
            Login to view your orders
          </h2>

          <p className="text-gray-500 mt-2">
            Your order history will appear here after login.
          </p>
        </div>
      </section>
    )
  }

  return (
    <section className="max-w-5xl mx-auto px-6 pb-20">

      <div className="flex items-center justify-between mb-8">

        <div>
          <h2 className="text-3xl font-bold text-gray-800">
            My Orders
          </h2>

          <p className="text-gray-500 mt-2">
            View your previous orders and track their status.
          </p>
        </div>

        <button
          onClick={loadOrders}
          className="border border-orange-600 text-orange-600
                     px-5 py-2 rounded-lg hover:bg-orange-50"
        >
          Refresh
        </button>

      </div>


      {/* Loading */}

      {loading && (
        <div className="bg-white rounded-2xl shadow-md p-8 text-center">
          <p className="text-gray-600">
            Loading your orders...
          </p>
        </div>
      )}


      {/* Error */}

      {!loading && error && (
        <div className="bg-red-100 text-red-700 rounded-lg p-4">
          {error}
        </div>
      )}


      {/* No orders */}

      {!loading && !error && orders.length === 0 && (
        <div className="bg-white rounded-2xl shadow-md p-10 text-center">

          <div className="text-5xl mb-4">
            🛍️
          </div>

          <h3 className="text-xl font-bold text-gray-800">
            No orders yet
          </h3>

          <p className="text-gray-500 mt-2">
            Your completed orders will appear here.
          </p>

        </div>
      )}


      {/* Orders */}

      {!loading && !error && orders.length > 0 && (
        <div className="space-y-8">

          {orders
            .slice()
            .reverse()
            .map((order) => {

              const currentStatusIndex =
                statusSteps.indexOf(order.status)

              const isCancelled =
                order.status === "CANCELLED"

              return (
                <div
                  key={order.id}
                  className="bg-white rounded-2xl shadow-md p-6"
                >

                  {/* Order Header */}

                  <div className="flex flex-col md:flex-row
                                  md:items-center
                                  md:justify-between
                                  gap-4
                                  border-b pb-5"
                  >

                    <div>
                      <h3 className="text-xl font-bold text-gray-800">
                        Order #{order.id}
                      </h3>

                      <p className="text-gray-500 mt-1">
                        Payment: {order.paymentMethod}
                      </p>

                      <p className="text-gray-500">
                        Delivery: {order.address}
                      </p>
                    </div>

                    <div className="text-left md:text-right">

                      <p className="text-2xl font-bold text-orange-600">
                        ₹{order.total}
                      </p>

                      <span
                        className={`inline-block mt-2 px-3 py-1
                                    rounded-full text-sm font-semibold
                                    ${
                                      isCancelled
                                        ? "bg-red-100 text-red-700"
                                        : order.status === "DELIVERED"
                                          ? "bg-green-100 text-green-700"
                                          : "bg-orange-100 text-orange-700"
                                    }`}
                      >
                        {order.status}
                      </span>

                    </div>

                  </div>


                  {/* Order Items */}

                  <div className="py-5">

                    <h4 className="font-bold text-gray-800 mb-3">
                      Items
                    </h4>

                    <div className="space-y-2">

                      {order.items.map((item) => (
                        <div
                          key={item.id}
                          className="flex justify-between
                                     text-gray-600"
                        >

                          <span>
                            {item.name} × {item.quantity}
                          </span>

                          <span className="font-semibold">
                            ₹{item.price * item.quantity}
                          </span>

                        </div>
                      ))}

                    </div>

                  </div>


                  {/* Cancel Order */}

                  {(order.status === "PLACED" ||
                    order.status === "CONFIRMED") && (
                    <div className="border-t pt-5">

                      <button
                        onClick={() =>
                          handleCancelOrder(order.id)
                        }
                        className="bg-red-600 text-white
                                   px-5 py-2 rounded-lg
                                   font-semibold
                                   hover:bg-red-700"
                      >
                        Cancel Order
                      </button>

                    </div>
                  )}



                  {/* Tracking */}

                  {!isCancelled && (
                    <div className="border-t pt-6">

                      <h4 className="font-bold text-gray-800 mb-6">
                        Order Tracking
                      </h4>

                      <div className="space-y-5">

                        {statusSteps.map(
                          (status, index) => {

                            const completed =
                              currentStatusIndex >= index

                            const current =
                              order.status === status

                            return (
                              <div
                                key={status}
                                className="flex items-center gap-4"
                              >

                                <div
                                  className={`w-8 h-8 rounded-full
                                              flex items-center
                                              justify-center
                                              text-sm font-bold
                                              ${
                                                completed
                                                  ? "bg-orange-600 text-white"
                                                  : "bg-gray-200 text-gray-500"
                                              }`}
                                >
                                  {completed
                                    ? "✓"
                                    : index + 1}
                                </div>

                                <div>

                                  <p
                                    className={`font-semibold ${
                                      current
                                        ? "text-orange-600"
                                        : completed
                                          ? "text-gray-800"
                                          : "text-gray-400"
                                    }`}
                                  >
                                    {status.replaceAll(
                                      "_",
                                      " "
                                    )}
                                  </p>

                                  {current && (
                                    <p className="text-sm text-orange-600">
                                      Current status
                                    </p>
                                  )}

                                </div>

                              </div>
                            )
                          }
                        )}

                      </div>

                    </div>
                  )}


                  {/* Cancelled */}

                  {isCancelled && (
                    <div className="border-t pt-5">

                      <div className="bg-red-50 text-red-700
                                      rounded-lg p-4">
                        This order has been cancelled.
                      </div>

                    </div>
                  )}

                </div>
              )
            })}

        </div>
      )}

    </section>
  )
}

export default Orders